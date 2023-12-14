//
// Created by qiuyudai on 2023/12/1.
//

#include "TimerQueue.h"

#include <cassert>
#include <cstring>
#include <sys/timerfd.h>
#include <memory>

#include "utils/Logger.h"
#include "TimerId.h"
#include "EventLoop.h"

int createTimerfd() {
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
                                   TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0) {
        LOG_FATAL << "Failed in timerfd_create";
    }
    return timerfd;
}

void readTimerfd(int timerfd, Timestamp now) {
    uint64_t howmany;
    ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
    std::cout << "TimerQueue::handleRead() " << howmany << " at " << now.toString() << '\n';
    if (n != sizeof howmany) {
        std::cerr << "ERROR: TimerQueue::handleRead() reads " << n << " bytes instead of 8\n";
    }
}

struct timespec howMuchTimeFromNow(Timestamp when) {
    int64_t microseconds = when.microSecondsSinceEpoch()
                           - Timestamp::now().microSecondsSinceEpoch();
    if (microseconds < 100) {
        microseconds = 100;
    }
    struct timespec ts{};
    ts.tv_sec = static_cast<time_t>(
            microseconds / Timestamp::kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<long>(
            (microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
    return ts;
}

void resetTimerfd(int timerfd, Timestamp expiration) {
    // wake up loop by timerfd_settime()
    struct itimerspec newValue;
    struct itimerspec oldValue;
    bzero(&newValue, sizeof newValue);
    bzero(&oldValue, sizeof oldValue);
    newValue.it_value = howMuchTimeFromNow(expiration);
    int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
    if (ret) {
        std::cerr << "ERROR: timerfd_settime()\n";
    }
}

TimerQueue::TimerQueue(EventLoop *loop) :
        loop_(loop),
        timerfd_(createTimerfd()),
        timerfdChannel_(loop_, timerfd_),
        timers_(),
        callingExpiredTimes_(false) {
    timerfdChannel_.setReadCallback((const Channel::ReadEventCallback &) [this] { handleRead(); });
    timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue() {
    // timerfdChannel_.disableAll();
    // timerfdChannel_.remove();
    ::close(timerfd_);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now) {
    std::vector<Entry> expired;
    Entry sentry(now, nullptr);

    auto it = timers_.lower_bound(sentry);
    assert(it == timers_.end() || now < it->first);

    // 从 set 中移除所有过期的 timer，并添加到 expired 向量中
    auto it0 = timers_.begin();
    while (it0 != it && it0->first < now) {
        // 由于 set 中的元素是常量，不能直接移动 unique_ptr
        // 需要先复制 key，然后删除原元素，再构造新元素
        expired.emplace_back(it0->first, std::move(const_cast<std::unique_ptr<Timer> &>(it0->second)));
        // 注意：const_cast 只应在这种特定情况下使用，并且需要非常小心
        it0 = timers_.erase(it0); // erase 会返回下一个有效迭代器
    }

    return expired;
}


TimerId TimerQueue::addTimer(const TimerCallback &cb, Timestamp when, double interval) {
    auto timer = std::make_unique<Timer>(cb, when, interval);
    Timer *timerPtr = timer.get(); // 保留裸指针用于返回 TimerId

    // 使用 lambda 捕获移动的 unique_ptr
    loop_->runInLoop([this, &timer]() mutable { addTimerInLoop(std::move(timer)); });

    return {timerPtr, timerPtr->sequence()};
}

void TimerQueue::addTimerInLoop(std::unique_ptr<Timer> timer) {
    loop_->assertInLoopThread();

    if (const bool earliestChanged = insert(timer.get()); earliestChanged)
        resetTimerfd(timerfd_, timer->expiration());
}

void TimerQueue::handleRead() {
    loop_->assertInLoopThread();
    Timestamp now(Timestamp::now());
    readTimerfd(timerfd_, now);
    auto expired = getExpired(now);

    callingExpiredTimes_ = true;
    cancelingTimers_.clear();
    for (const auto &entry: expired)
        entry.second->run();
    callingExpiredTimes_ = false;

    reset(expired, now);
}

void TimerQueue::reset(const std::vector<Entry> &expired, Timestamp now) {
    Timestamp nextExpire;

    for (const Entry &it: expired) {
        ActiveTimer timer(it.second.get(), it.second->sequence());
        if (it.second->repeat() && !cancelingTimers_.contains(timer)) {
            it.second->restart(now);
            // 由于unique_ptr无法复制，需要使用std::move来转移所有权
            insert(it.second.get());
        }
        // 不再需要 delete，unique_ptr 会自动管理内存
    }

    if (!timers_.empty()) {
        nextExpire = timers_.begin()->second->expiration();
    }

    if (nextExpire.valid()) {
        resetTimerfd(timerfd_, nextExpire);
    }
}

bool TimerQueue::insert(Timer *timer) {
    loop_->assertInLoopThread();
    assert(timers_.size() == activeTimers_.size());
    bool earliestChanged = false;
    Timestamp when = timer->expiration();
    auto it = timers_.begin();
    if (it == timers_.end() || when < it->first)
        earliestChanged = true;
    {
        auto seq = timer->sequence();
        auto result = activeTimers_.insert(ActiveTimer(timer, seq));
        assert(result.second);
        (void) result;
    }

    assert(timers_.size() == activeTimers_.size());
    return earliestChanged;
}

void TimerQueue::cancel(TimerId timer_id) {
    loop_->runInLoop(std::bind(&TimerQueue::cancelingTimers_, this, timer_id));
}

void TimerQueue::cancelInLoop(TimerId timerId) {
    loop_->assertInLoopThread();
    assert(timers_.size() == activeTimers_.size());
    ActiveTimer timer(timerId.timer_, timerId.sequence_);
    auto it = activeTimers_.find(timer);
    if (it != activeTimers_.end()) {
        auto n = timers_.erase(Entry(it->first->expiration(), it->first));
        assert(n == 1);
        (void) n;
        activeTimers_.erase(it);
    } else if (callingExpiredTimes_)
        cancelingTimers_.emplace(timer);
    assert(timers_.size() == activeTimers_.size());
}

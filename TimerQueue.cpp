//
// Created by qiuyudai on 2023/12/1.
//

#include "TimerQueue.h"

#include <cassert>
#include <cstring>
#include <sys/timerfd.h>

int createTimerfd() {
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
                                   TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0) {
        LOG_SYSFATAL << "Failed in timerfd_create";
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
    struct timespec ts;
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
    timerfdChannel_.setReadCallback([this] {
        handleRead();
    });
    timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue() {
    // timerfdChannel_.disableAll();
    // timerfdChannel_.remove();
    ::close(timerfd_);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now) {
    std::vector<Entry> expired;
    const auto sentry = std::make_pair(now, nullptr);

    const auto it = timers_.lower_bound(sentry);
    assert(it == timers_.end() || now < it->first);
    std::copy(timers_.begin(), it, std::back_inserter(expired));
    timers_.erase(timers_.begin(), it);

    return expired;
}

TimerId TimerQueue::addTimer(const TimerCallback &cb, Timestamp when, double interval) {
    auto timer = std::make_unique<Timer>(cb, when, interval);
    Timer *timerPtr = timer.get(); // 保留裸指针用于返回 TimerId

    // 使用 lambda 捕获移动的 unique_ptr
    loop_->runInLoop([this, t = std::move(timer)]() mutable { addTimerInLoop(std::move(t)); });

    return {timerPtr, timerPtr->sequence()};
}

void TimerQueue::addTimerInLoop(std::unique_ptr<Timer> &&timer) {
    loop_->assertInLoopThread();

    if (const bool earliestChanged = insert(std::move(timer)); earliestChanged)
        resetTimerfd(timerfd_, timer->expiration());
}

void TimerQueue::handleRead() {
    loop_->assertInLoopThread();
    Timestamp now(Timestamp::now());
    readTimerfd(timerfd_, now);
    auto expired = getExpired(now);

    callingExpiredTimes_ = true;
    for (const auto& entry : expired)
        entry.second->run();
    callingExpiredTimes_ = false;

    reset(expired, now);
}

void TimerQueue::reset(const std::vector<Entry> &expired, Timestamp now) {
    Timestamp nextExpired;
    for (const auto& it : expired) {
        auto seq = it.second->sequence();
        ActiveTimer timer(std::move(it.second), seq);
        if (timer.first->repeat()) {
            // todo
        }
    }
}

bool TimerQueue::insert(std::unique_ptr<Timer> &&timer) {
    loop_->assertInLoopThread();
    assert(timers_.size() == activeTimers_.size());
    bool earliestChanged = false;
    Timestamp when = timer->expiration();
    auto it = timers_.begin();
    if (it == timers_.end() || when < it->first)
        earliestChanged = true;
    {
        auto result = activeTimers_.insert(ActiveTimer(timer, timer->sequence()));
        assert(result.second);
        (void)result;
    }

    assert(timers_.size() == activeTimers_.size());
    return earliestChanged;
}
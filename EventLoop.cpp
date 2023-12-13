#include "EventLoop.h"

#include <cassert>
#include <iostream>

#include "Poller.h"
#include "TimerQueue.h"
#include "utils/Logger.h"

const int EventLoop::kPollTimeMs = 5 * 1000;

thread_local EventLoop* t_loopInThisThread = nullptr;

EventLoop::EventLoop() : looping_(false), threadId_(std::this_thread::get_id()), poller_(new Poller(this))
{
    std::clog << "EventLoop created " << this << " in thread " << threadId_ << std::endl;

    if (t_loopInThisThread)
    {
        std::cerr << "ERROR: Another EventLoop " << t_loopInThisThread << " exists in this thread " << threadId_ << std::endl;
        // std::cerr << "Another EventLoop " << t_loopInThisThread <<  exists in this thread {}.\n", t_loopInThisThread, threadId_);
        pthread_exit(nullptr);
    }
    else 
        t_loopInThisThread = this;
}

EventLoop::~EventLoop()
{
    assert(!looping_);
    t_loopInThisThread = nullptr;
}

EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
    return t_loopInThisThread;
}

// TODO：首先等5秒退出
void EventLoop::loop(Timestamp time)
{
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    quit_ = false;

    while (!quit_)
    {
        activeChannels_.clear();
        poller_->poll(kPollTimeMs, &activeChannels_);
        for (const auto& channel : activeChannels_)
            channel->handleEvent(time);
        doPendingFunctors();
    }

    std::clog << "EventLoop " << this << " stop looping\n";
    looping_ = false;
}

void EventLoop::abortNotInLoopThread() const
{
    LOG_ERROR << "EventLoop::abortNotInLoopThread - EventLoop " << this
            << " was created in threadId_ = " << threadId_
            << ", current thread id = " <<  std::this_thread::get_id() << '\n';
}

void EventLoop::quit()
{
    quit_ = true;
    if (!isInLoopThread())
        wakeup();
}

void EventLoop::updateChannel(Channel* channel) const
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->updateChannel(channel);
}

// void EventLoop::setPoller(Poller *p) {
//     poller_.reset(p);
// }

TimerId EventLoop::runAt(const Timestamp& time, const TimerCallback& cb) const
{
    return timerQueue_->addTimer(cb, time, 0.0);
}

TimerId EventLoop::runAfter(const double delay, const TimerCallback& cb) const
{
    const Timestamp time(addTime(Timestamp::now(), delay));
    return runAt(time, cb);
}

TimerId EventLoop::runEvery(const double interval, const TimerCallback& cb) const
{
    const Timestamp time(addTime(Timestamp::now(), interval));
    return timerQueue_->addTimer(cb, time, interval);
}

void EventLoop::runInLoop(const Functor& cb)
{
    if (isInLoopThread())
        cb();
    else
        queueInLoop(cb);
}

void EventLoop::queueInLoop(const Functor& cb)
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(cb);
    }

    if (!isInLoopThread() || callingPendingFunctors)
        wakeup();
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    callingPendingFunctors = true;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for (auto& fun : functors)
        fun();
    callingPendingFunctors = false;
}

void EventLoop::wakeup() const
{
    constexpr uint64_t one = 1;
    if (const auto len = write(wakeupFd_, &one, sizeof one); len != sizeof one)
    {
        std::cerr << "ERROR: EventLoop::wakeup writes " << len << " instead of " << sizeof(one) << '\n';
    }
}

void EventLoop::handleRead() const
{
    uint64_t one;
    if (const auto n = read(wakeupFd_, &one, sizeof one); n != sizeof one)
    {
        std::cerr << "ERROR: EventLoop::handleRead read " << n << " instead of " << sizeof(one) << '\n';
    }
}


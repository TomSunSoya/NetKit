//
// Created by qiuyudai on 2023/11/29.
//

#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <boost/noncopyable.hpp>
#include <atomic>
#include <thread>
#include <memory>
#include <iostream>
#include <vector>
#include <mutex>

#include "Callbacks.h"
#include "TimerId.h"
#include "Timestamp.h"


class Channel;
class Poller;
class TimerQueue;

class EventLoop : public boost::noncopyable {
public:
    EventLoop();
    ~EventLoop();

    void loop(Timestamp time = Timestamp::now());
    void assertInLoopThread() const
    {
        if (!isInLoopThread())
        {
            abortNotInLoopThread();
        }
    }

    [[nodiscard]] bool isInLoopThread() const
    {
        return threadId_ == std::this_thread::get_id();
    }

    void quit();
    void updateChannel(Channel* channel) const;
    void runInLoop(const Functor& cb);

    // void setPoller(Poller* poll);

    TimerId runAt(const Timestamp& time, const TimerCallback& cb) const;
    TimerId runAfter(double delay, const TimerCallback& cb) const;
    TimerId runEvery(double interval, const TimerCallback& cb) const;

    static EventLoop* getEventLoopOfCurrentThread();
    void queueInLoop(const Functor& cb);
    void removeChannel(Channel* channel);

private:
    void abortNotInLoopThread() const;
    void handleRead() const;
    void wakeup() const;
    void doPendingFunctors();


    using ChannelList = std::vector<Channel*>;

    static const int kPollTimeMs;

    std::atomic_bool looping_;
    std::atomic_bool quit_;
    std::atomic_bool callingPendingFunctors;
    std::unique_ptr<Poller> poller_;
    ChannelList activeChannels_;
    const std::thread::id threadId_;
    Timestamp pollReturnTime;
    std::unique_ptr<TimerQueue> timerQueue_{};
    int wakeupFd_{};
    std::unique_ptr<Channel> wakeupChannel_;
    std::mutex mutex_;
    std::vector<Functor> pendingFunctors_;      // @GuardedBy mutex_
};

#endif //EVENTLOOP_H
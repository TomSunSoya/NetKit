//
// Created by qiuyudai on 2023/12/1.
//

#ifndef TIMERQUEUE_H
#define TIMERQUEUE_H

#include "Timer.h"
#include <algorithm>
#include <memory>
#include <set>
#include <vector>
#include <boost/noncopyable.hpp>
#include <unordered_set>

#include "EventLoop.h"
#include "TimerId.h"
#include "Callbacks.h"

class TimerQueue : public boost::noncopyable{
public:
    explicit TimerQueue(EventLoop *loop);
    ~TimerQueue();

    /*
     * Schedules the callback to be run at given time.
     * Repeats if @interval > 0.0.
     * Must be thread safe. Usually be called from other thread.
     */
    TimerId addTimer(const TimerCallback& cb, Timestamp when, double interval);

    // void cancel(TimerId timer_id);

private:
    using Entry = std::pair<Timestamp, std::unique_ptr<Timer>>;
    using TimerList = std::set<Entry>;
    using ActiveTimer = std::pair<std::unique_ptr<Timer>, int64_t>;
    using ActiveTimerSet = std::unordered_set<ActiveTimer>;

    // called when timerfd alarms
    void handleRead();
    void addTimerInLoop(std::unique_ptr<Timer>&& timer);

    // move out all expired timers
    std::vector<Entry> getExpired(Timestamp now);
    void reset(const std::vector<Entry>& expired, Timestamp now);

    bool insert(std::unique_ptr<Timer>&& timer);

    EventLoop* loop_;
    const int timerfd_;
    Channel timerfdChannel_;
    // Timer list sorted by expiration
    TimerList timers_;
    std::atomic_bool callingExpiredTimes_;
    ActiveTimerSet activeTimers_;
    ActiveTimerSet cancelingTimers_;
};



#endif //TIMERQUEUE_H

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

    // called when timerfd alarms
    void handleRead();

    // move out all expired timers
    std::vector<Entry> getExpired(Timestamp now);
    void reset(const std::vector<Entry>& expired, Timestamp now);

    bool insert(Timer *timer);

    EventLoop* loop_;
    const int timerfd_;
    Channel timerfdChannel_;
    // Timer list sorted by expiration
    TimerList timers_;
};



#endif //TIMERQUEUE_H

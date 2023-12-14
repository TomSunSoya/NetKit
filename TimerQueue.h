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
#include <atomic>
#include <unordered_set>
#include <utility>

#include "Callbacks.h"
#include "Channel.h"


class EventLoop;
class Timer; // 假设 Timer 类已经定义
class TimerId;

struct TimerHash {
    size_t operator()(const std::pair<Timer*, long int>& timer) const {
        return std::hash<Timer*>()(timer.first) ^ std::hash<long int>()(timer.second);
    }
};

struct TimerEqual {
    bool operator()(const std::pair<Timer*, long int>& a, const std::pair<Timer*, long int>& b) const {
        return a.first == b.first && a.second == b.second;
    }
};


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
    void addTimerInLoop(std::unique_ptr<Timer> timer);

     void cancel(TimerId timer_id);

private:
    using Entry = std::pair<Timestamp, std::unique_ptr<Timer>>;
    using TimerList = std::set<Entry>;
    using ActiveTimer = std::pair<Timer*, int64_t>;
    using ActiveTimerSet = std::unordered_set<ActiveTimer, TimerHash, TimerEqual>;

    // called when timerfd alarms
    void handleRead();

    // move out all expired timers
    std::vector<Entry> getExpired(Timestamp now);
    void reset(const std::vector<Entry>& expired, Timestamp now);
    void cancelInLoop(TimerId timerId);

    bool insert(Timer* timer);

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

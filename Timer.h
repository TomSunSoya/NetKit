//
// Created by qiuyudai on 2023/12/1.
//

#ifndef TIMER_H
#define TIMER_H

#include "Timestamp.h"
#include <atomic>
#include <functional>
#include <boost/noncopyable.hpp>

#include "Callbacks.h"

class Timer : boost::noncopyable {
public:
    Timer(TimerCallback cb, const Timestamp when, double interval) :
        callback_(std::move(cb)), expiration_(when), interval_(interval), repeat_(interval_ > 0.0), sequence_(++s_numCreated_) {}
    void run() const
    {
        callback_();
    }

    void restart(Timestamp now);

    [[nodiscard]] Timestamp expiration() const
    {
        return expiration_;
    }

    [[nodiscard]] bool repeat() const
    {
        return repeat_;
    }

    [[nodiscard]] int64_t sequence() const
    {
        return sequence_;
    }

    static int64_t numCreated()
    {
        return s_numCreated_;
    }

private:
    const TimerCallback callback_;
    Timestamp expiration_;
    const double interval_;
    const bool repeat_;
    const int64_t sequence_;

    static std::atomic_int64_t s_numCreated_;
};



#endif //TIMER_H

#ifndef TIMERID_H
#define TIMERID_H

#include <boost/noncopyable.hpp>

class Timer;

class TimerId : boost::noncopyable {
public:
    TimerId() : timer_(nullptr), sequence_(0) {}

    TimerId(Timer* timer, int64_t sequence)
        : timer_(timer),
          sequence_(sequence)
    {
    }

    friend class TimerQueue;
private:
    Timer* timer_;
    int64_t sequence_;
};



#endif //TIMERID_H

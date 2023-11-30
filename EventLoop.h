//
// Created by qiuyudai on 2023/11/29.
//

#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <boost/log/expressions.hpp>
#include <atomic>
#include <thread>

class EventLoop : public boost::noncopyable {
public:
    EventLoop();
    ~EventLoop();

    void loop();
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

    static EventLoop* getEventLoopOfCurrentThread();

private:
    void abortNotInLoopThread() const;

    std::atomic_bool looping_;
    const std::thread::id threadId_;
};



#endif //EVENTLOOP_H

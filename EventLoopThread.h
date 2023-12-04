//
// Created by qiuyudai on 2023/12/4.
//

#ifndef NETKIT_EVENTLOOPTHREAD_H
#define NETKIT_EVENTLOOPTHREAD_H

#include <boost/noncopyable.hpp>
#include "EventLoop.h"

class EventLoopThread : public boost::noncopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    explicit EventLoopThread(ThreadInitCallback  cb = ThreadInitCallback(),
                    const std::string& name = std::string());
    ~EventLoopThread();
    EventLoop* startLoop();
private:
    void threadFunc();
    EventLoop* loop_;
    bool existing_;
    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    ThreadInitCallback callback_;
};


#endif //NETKIT_EVENTLOOPTHREAD_H

//
// Created by qiuyudai on 2023/12/14.
//

#ifndef NETKIT_EVENTLOOPTHREADPOOL_H
#define NETKIT_EVENTLOOPTHREADPOOL_H

#include <boost/noncopyable.hpp>
#include <vector>
#include <memory>
#include <functional>

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : public boost::noncopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThreadPool(EventLoop* baseLoop, const std::string& nameArg);
    ~EventLoopThreadPool();

    void setThreadNum(int numThreads) {
        numThreads_ = numThreads;
    }
    void start(const ThreadInitCallback& cb = ThreadInitCallback());
    EventLoop* getNextLoop();

    [[nodiscard]] bool started() const {
        return started_;
    }
    [[nodiscard]] const std::string& name() const {
        return name_;
    }

private:
    EventLoop* baseLoop_;
    std::string name_;
    bool started_;
    int numThreads_;
    int next_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;
};


#endif //NETKIT_EVENTLOOPTHREADPOOL_H

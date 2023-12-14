//
// Created by qiuyudai on 2023/12/14.
//

#include "EventLoopThreadPool.h"
#include "EventLoop.h"
#include "EventLoopThread.h"

#include <cassert>

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop, const std::string &name) :
        baseLoop_(baseLoop), name_(name), started_(false), next_(0), numThreads_(0) {

}

EventLoopThreadPool::~EventLoopThreadPool() {
    // DO NOT delete loop, it's a stack variable
}

void EventLoopThreadPool::start(const EventLoopThreadPool::ThreadInitCallback &cb) {
    assert(!started_);
    baseLoop_->assertInLoopThread();

    started_ = true;

    for (int i = 0; i < numThreads_; ++i) {
        char buf[name_.size() + 32];
        snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);
        auto *t = new EventLoopThread(cb, buf);
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        loops_.push_back(t->startLoop());
    }
    if (numThreads_ == 0 && cb) {
        cb(baseLoop_);
    }
}

EventLoop *EventLoopThreadPool::getNextLoop() {
    baseLoop_->assertInLoopThread();
    assert(started_);
    auto loop = baseLoop_;

    if (!loops_.empty()) {
        loop = loops_.at(next_);
        ++next_;
        if (static_cast<size_t>(next_) >= loops_.size())
            next_ = 0;
    }
    return loop;
}

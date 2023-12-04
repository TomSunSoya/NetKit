//
// Created by qiuyudai on 2023/12/4.
//

#include "EventLoopThread.h"
#include <thread>
#include <condition_variable>
#include <functional>
#include <utility>
#include <cassert>

EventLoopThread::EventLoopThread(EventLoopThread::ThreadInitCallback cb, const std::string &name) :
        loop_(nullptr), existing_(false), thread_(std::bind(&EventLoopThread::threadFunc, this), name),
        mutex_(), cond_(), callback_(std::move(cb)), startSignal_(), startFuture_(startSignal_.get_future()) {
}

EventLoopThread::~EventLoopThread() {
    existing_ = true;
    if (loop_ != nullptr) {
        loop_->quit();
        thread_.join();
    }
}

EventLoop *EventLoopThread::startLoop() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        assert(loop_ == nullptr);
    }

    // start thread
    startSignal_.set_value();
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this] { return loop_ != nullptr; });
    }
    return loop_;
}

void EventLoopThread::threadFunc() {
    // wait for startLoop()
    startFuture_.wait();

    EventLoop loop;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }
    loop.loop();
}

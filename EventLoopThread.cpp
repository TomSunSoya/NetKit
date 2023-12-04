//
// Created by qiuyudai on 2023/12/4.
//

#include "EventLoopThread.h"
#include <thread>
#include <condition_variable>
#include <functional>
#include <utility>

EventLoopThread::EventLoopThread(EventLoopThread::ThreadInitCallback cb, const std::string &name) :
        loop_(nullptr), existing_(false), thread_(std::bind(&EventLoopThread::threadFunc, this), name),
        mutex_(), cond_(), callback_(std::move(cb)) {
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
        std::unique_lock<std::mutex> lock(mutex_);
        while (loop_ == nullptr)
            cond_.wait(lock);
    }
    return loop_;
}

void EventLoopThread::threadFunc() {
    EventLoop loop;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_all();
    }
    loop.loop();
}

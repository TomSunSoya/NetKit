//
// Created by qiuyudai on 2023/11/29.
//

#include "EventLoop.h"
#include <iostream>
#include <poll.h>

__thread EventLoop* t_loopInThisThread = nullptr;

EventLoop::EventLoop() : looping_(false), threadId_(std::this_thread::get_id())
{
    std::clog << "EventLoop created " << this << " in thread " << threadId_ << std::endl;

    if (t_loopInThisThread)
    {
        std::cerr << "Another EventLoop " << t_loopInThisThread << " exists in this thread " << threadId_ << std::endl;
        pthread_exit(nullptr);
    }
    else
        t_loopInThisThread = this;
}

EventLoop::~EventLoop()
{
    assert(!looping_);
    t_loopInThisThread = nullptr;
}

EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
    return t_loopInThisThread;
}

// TODO：首先等5秒退出
void EventLoop::loop()
{
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;

    ::poll(nullptr, 0, 5 * 1000);

    std::clog << "EventLoop " << this << " stop looping\n";
    looping_ = false;
}

void EventLoop::abortNotInLoopThread() const
{
    std::cerr << "EventLoop::abortNotInLoopThread - EventLoop " << this
            << " was created in threadId_ = " << threadId_
            << ", current thread id = " <<  std::this_thread::get_id() << '\n';
}




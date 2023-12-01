//
// Created by qiuyudai on 2023/11/29.
//

#include "EventLoop.h"

#include <cassert>
#include <iostream>

#include "Poller.h"

const int EventLoop::kPollTimeMs = 5 * 1000;

thread_local EventLoop* t_loopInThisThread = nullptr;

EventLoop::EventLoop() : looping_(false), threadId_(std::this_thread::get_id()), poller_(new Poller(this))
{
    std::clog << "EventLoop created " << this << " in thread " << threadId_ << std::endl;

    if (t_loopInThisThread)
    {
        std::cerr << "Another EventLoop " << t_loopInThisThread << " exists in this thread " << threadId_ << std::endl;
        // std::cerr << "Another EventLoop " << t_loopInThisThread <<  exists in this thread {}.\n", t_loopInThisThread, threadId_);
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
    quit_ = false;

    while (!quit_)
    {
        activeChannels_.clear();
        poller_->poll(kPollTimeMs, &activeChannels_);
        for (const auto& channel : activeChannels_)
            channel->handleEvent();
    }

    std::clog << "EventLoop " << this << " stop looping\n";
    looping_ = false;
}

void EventLoop::abortNotInLoopThread() const
{
    std::cerr << "ERROR: EventLoop::abortNotInLoopThread - EventLoop " << this
            << " was created in threadId_ = " << threadId_
            << ", current thread id = " <<  std::this_thread::get_id() << '\n';
}

void EventLoop::quit()
{
    quit_ = true;
    // wakeup()
}

void EventLoop::updateChannel(Channel* channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->updateChannel(channel);
}

// void EventLoop::setPoller(Poller *p) {
//     poller_.reset(p);
// }


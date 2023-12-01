//
// Created by qiuyudai on 2023/11/30.
//

#include "Channel.h"

#include <iostream>

#include "EventLoop.h"
#include <poll.h>
#include <sys/types.h>

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, const int fd) : loop_(loop), fd_(fd), events_(0), revents_(0), index_(-1)
{
}

void Channel::update()
{
    loop_->updateChannel(this);
}

void Channel::handleEvent() const
{
    if (revents_ & POLLNVAL)
        std::clog << "WARN: Channel::handle_event() POLLNVAL\n";

    if (revents_ & (POLLERR | POLLNVAL))
    {
        if (errorCallback_) errorCallback_();
    }
    if (revents_ & (POLLIN | POLLPRI | POLLHUP))
    {
        if (readCallback_) readCallback_();
    }
    if (revents_ & POLLOUT)
    {
        if (writeCallback_) writeCallback_();
    }
}




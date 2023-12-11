//
// Created by qiuyudai on 2023/11/30.
//

#include "Channel.h"

#include <iostream>

#include "EventLoop.h"
#include "utils/Logger.h"
#include <poll.h>
#include <sys/types.h>
#include <cassert>

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, const int fd) : loop_(loop), fd_(fd), events_(0), revents_(0), index_(-1), eventHandling_(
        false)
{
}

void Channel::update()
{
    loop_->updateChannel(this);
}

void Channel::handleEvent() const
{
    eventHandling_ = false;
    if (revents_ & POLLNVAL)
        LOG_INFO << "WARN: Channel::handle_event() POLLNVAL\n";

    if ((revents_ & (POLLHUP)) && !(revents_ & POLLIN)) {
        LOG_WARN << "Channel::handle_event() POLLHUP";
        if (closeCallback_) closeCallback_();
    }

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
    eventHandling_ = false;
}

Channel::~Channel() {
    assert(!eventHandling_);
}




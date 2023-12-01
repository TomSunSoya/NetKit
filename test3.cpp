//
// Created by qiuyudai on 2023/11/30.
//
#include <sys/timerfd.h>
#include <iostream>
#include <cstring>

#include "EventLoop.h"
#include "Channel.h"
#include "Poller.h"

EventLoop* g_loop;

int main()
{
    EventLoop loop;
    g_loop = &loop;
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    Channel channel(&loop, timerfd);
    channel.setReadCallback([]
    {
        std::cout << "Timeout!\n";
        g_loop->quit();
    });

    channel.enableReading();

    struct itimerspec howlong;
    bzero(&howlong, sizeof howlong);
    howlong.it_value.tv_sec = 5;
    ::timerfd_settime(timerfd, 0, &howlong, nullptr);

    loop.loop();
    ::close(timerfd);
    
}
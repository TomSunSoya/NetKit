//
// Created by qiuyudai on 2023/11/30.
//
#include "EventLoop.h"
#include <thread>

EventLoop* g_loop;

void threadFun()
{
    EventLoop loop, loop2;
    loop.loop();
    loop2.loop();
}

int main()
{
    std::thread thread(threadFun);
    thread.join();

    return 0;
}
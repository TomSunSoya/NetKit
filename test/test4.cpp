//
// Created by qiuyudai on 2023/12/1.
// TimerQueue test

#include <thread>
#include <iostream>
#include <functional>
#include <unistd.h>

#include "../EventLoop.h"
#include "../TimerQueue.h"

int cnt = 0;
EventLoop *g_loop;

void printTid()
{
    std::cout << "pid = " << getpid() << ", tid = " << std::this_thread::get_id() << '\n';
    std::cout << "Now " << Timestamp::now().toString() << '\n';
}

void print(const std::string& msg)
{
    std::cout << "msg: " << Timestamp::now().toString() << " " << msg << '\n';
    if (++cnt == 20) g_loop->quit();
}

// void cancel(TimerId timer_id)
// {
//     loop->can
// }

int main()
{
    printTid();
    sleep(1);
    {
        EventLoop loop;
        g_loop = &loop;
        print("main");

        loop.runAfter(1, [] { return print("once1"); });
        loop.runAfter(1.5, [] { return print("once1.5"); });
        loop.runAfter(2.5, [] { return print("once2.5"); });
        loop.runAfter(3.5, [] { return print("once3.5"); });
        TimerId t45 = loop.runAfter(4.5, std::bind(print, "once4.5"));
        loop.runAfter(4.2, [t45] { return print("cancel"); });
        loop.runAfter(4.8, [t45] { return print("cancel"); });
        loop.runEvery(2, [] { return print("every2"); });
        TimerId t3 = loop.runEvery(3, [] { return print("every3"); });
        loop.runAfter(9.001, [t3] { return print("cancel"); });
    }
}

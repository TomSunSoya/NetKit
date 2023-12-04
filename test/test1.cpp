//
// Created by qiuyudai on 2023/11/30.
//
#include "EventLoop.h"
#include <iostream>
#include <format>

void threadFunc()
{
    std::cout << "threadFunc(): pid = " << getpid() << ", tid = " << std::this_thread::get_id() << '\n';
    // std::cout << std::format("threadFunc(): pid = {}, tid = {}\n", getpid(), std::this_thread::get_id());
    EventLoop loop;
    loop.loop();
}

int main()
{
    //boost::log::register_simple_formatter_factory<boost::log::trivial::severity_level, char>("Severity");
    //boost::log::add_console_log();
    std::cout << "main(): pid = " << getpid() << ", tid = " << std::this_thread::get_id() << '\n';
    // logger.info("main(): pid = {}, tid = {}\n", getpid(), std::this_thread::get_id());

    EventLoop loop;
    std::thread th(threadFunc);

    loop.loop();
    th.join();

    return 0;
}
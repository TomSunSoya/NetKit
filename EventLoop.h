//
// Created by qiuyudai on 2023/11/29.
//

#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <boost/noncopyable.hpp>
#include <atomic>
#include <thread>
#include <memory>
#include <iostream>
#include <vector>

#include "Channel.h"

class Poller;

class EventLoop : public boost::noncopyable {
public:
    EventLoop();
    ~EventLoop();

    void loop();
    void assertInLoopThread() 
    {
        if (!isInLoopThread())
        {
            abortNotInLoopThread();
        }
    }

    [[nodiscard]] bool isInLoopThread() const
    {
        return threadId_ == std::this_thread::get_id();
    }

    void quit();
    void updateChannel(Channel* channel);
    // void setPoller(Poller* poll);

    static EventLoop* getEventLoopOfCurrentThread();

private:
    void abortNotInLoopThread() const;
    using ChannelList = std::vector<Channel*>;

    static const int kPollTimeMs;

    std::atomic_bool looping_;
    std::atomic_bool quit_;
    std::unique_ptr<Poller> poller_;
    ChannelList activeChannels_;
    const std::thread::id threadId_;
};

// template <>
// struct std::formatter<EventLoop*> : std::formatter<std::string> {
//     auto format(EventLoop* loop, format_context& ctx) {
//         std::string representation = "EventLoop@" + std::to_string(reinterpret_cast<std::uintptr_t>(loop));
//         return std::formatter<std::string>::format(representation, ctx);
//     }
// };



#endif //EVENTLOOP_H
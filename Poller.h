//
// Created by qiuyudai on 2023/11/30.
//

#ifndef POLLER_H
#define POLLER_H

#include <unordered_map>
#include <vector>
#include <poll.h>

#include "EventLoop.h"
#include "Timestamp.h"

class Channel;

class Poller : public boost::noncopyable {
// IO Multiplexing with poll(2)
// This class doesn't own the Channel objects.
public:
    using ChannelList = std::vector<Channel*>;

    explicit Poller(EventLoop *loop);
    ~Poller() = default;

    // Polls the I/O events;
    // Must be called in the loop thread
    Timestamp poll(int timeoutMs, ChannelList* activeChannels);

    // Changes the interested I/O events
    // Must be called in the loop thread
    void updateChannel(Channel* channel);

    void assertInLoopThread() const
    {
        ownerLoop_->assertInLoopThread();
    }

private:
    void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;

    using PollfdList = std::vector<struct pollfd>;
    using ChannelMap = std::unordered_map<int, Channel*>;

    EventLoop* ownerLoop_;
    PollfdList pollfds_;
    ChannelMap channels_;
};



#endif //POLLER_H

//
// Created by qiuyudai on 2023/11/30.
//

#include "Poller.h"
#include <poll.h>
#include <iostream>

#include "Channel.h"

Poller::Poller(EventLoop* loop) : ownerLoop_(loop)
{
}

Timestamp Poller::poll(const int timeoutMs, ChannelList* activeChannels)
{
    const int numEvents = ::poll(pollfds_.data(), pollfds_.size(), timeoutMs);
    const Timestamp now(Timestamp::now());
    if (numEvents > 0)
    {
        std::clog << numEvents << " events happended\n";
        fillActiveChannels(numEvents, activeChannels);
    } else if (numEvents == 0)
        std::clog << "Nothing happended.\n";
    else
        std::cerr << "ERROR: Poller::poll()\n";
    return now;
}

void Poller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const
{
    for (auto pfd = pollfds_.begin(); pfd != pollfds_.end() && numEvents > 0; ++pfd)
    {
        if (pfd->revents > 0)
        {
            --numEvents;
            auto ch = channels_.find(pfd->fd);
            assert(ch != channels_.end());
            auto channel = ch->second;
            assert((channel->fd() == pfd->fd));
            channel->set_revents(pfd->revents);
            // pfd->revents = 0;
            activeChannels->emplace_back(channel);
        }
    }
}

void Poller::updateChannel(Channel* channel)
{
    assertInLoopThread();
    std::clog << "fd = " << channel->fd() << " events = " << channel->events() << '\n';

    if (channel->index() < 0)
    {
        // a new one, add to pollfds_
        assert(channels_.contains(channel->fd()));

        struct pollfd pfd{};
        pfd.fd = channel->fd();
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        pollfds_.emplace_back(pfd);
        const int idx = static_cast<int>(pollfds_.size()) - 1;
        channel->set_index(idx);
        channels_[pfd.fd] = channel;
    } else
    {
        // update existing one
        assert(channels_.contains(channel->fd()));
        assert(channels_[channel->fd()] == channel);
        const int idx = channel->index();
        assert(idx >= 0 && idx < static_cast<int>(pollfds_.size()));

        auto& [fd, evts, revts] = pollfds_[idx];
        assert(fd == channel->fd() || fd == -1);
        evts = static_cast<short>(channel->events());
        revts = 0;
        if (channel->isNoneEvent())
            // ignore this pollfd
            fd = -1;
    }
}






//
// Created by qiuyudai on 2023/11/30.
//

#include "Poller.h"
#include <poll.h>
#include <iostream>
#include <cassert>

#include "Channel.h"
#include "utils/Logger.h"

Poller::Poller(EventLoop *loop) : ownerLoop_(loop) {
}

Timestamp Poller::poll(const int timeoutMs, ChannelList *activeChannels) {
    const int numEvents = ::poll(pollfds_.data(), pollfds_.size(), timeoutMs);
    const Timestamp now(Timestamp::now());
    if (numEvents > 0) {
        LOG_INFO << numEvents << " events happened";
        fillActiveChannels(numEvents, activeChannels);
    } else if (numEvents == 0)
        LOG_INFO << "Nothing happened.";
    else
        LOG_ERROR << "ERROR: Poller::poll()\n";
    return now;
}

void Poller::fillActiveChannels(int numEvents, ChannelList *activeChannels) const {
    for (auto pfd = pollfds_.begin(); pfd != pollfds_.end() && numEvents > 0; ++pfd) {
        if (pfd->revents > 0) {
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

void Poller::updateChannel(Channel *channel) {
    assertInLoopThread();
    LOG_INFO << "fd = " << channel->fd() << " events = " << channel->events();

    if (channel->index() < 0) {
        // a new one, add to pollfds_
        assert(!channels_.contains(channel->fd()));

        struct pollfd pfd{};
        pfd.fd = channel->fd();
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        pollfds_.emplace_back(pfd);
        const int idx = static_cast<int>(pollfds_.size()) - 1;
        channel->set_index(idx);
        channels_[pfd.fd] = channel;
    } else {
        // update existing one
        assert(channels_.contains(channel->fd()));
        assert(channels_[channel->fd()] == channel);
        const int idx = channel->index();
        assert(idx >= 0 && idx < static_cast<int>(pollfds_.size()));

        auto &[fd, evts, revts] = pollfds_[idx];
        assert(fd == channel->fd() || fd == -channel->fd() - 1);
        evts = static_cast<short>(channel->events());
        revts = 0;
        if (channel->isNoneEvent())
            // ignore this pollfd
            fd = -channel->fd() - 1;
    }
}

void Poller::removeChannel(Channel *channel) {
    assertInLoopThread();
    LOG_TRACE << "fd = " << channel->fd();
    assert(channels_.contains(channel->fd()));
    assert(channels_[channel->fd()] == channel);
    assert(channel->isNoneEvent());

    int idx = channel->index();
    assert(idx >= 0 && idx < static_cast<int>(pollfds_.size()));

    const auto &pfd = pollfds_[idx];
    (void) pfd;
    assert(pfd.fd == -channel->fd() - 1 && pfd.events == channel->events());
    size_t n = channels_.erase(channel->fd());
    assert(n == 1);
    (void) n;

    if (static_cast<size_t>(idx) == pollfds_.size() - 1)
        pollfds_.pop_back();
    else {
        int channelAtEnd = pollfds_.back().fd;
        std::iter_swap(pollfds_.begin() + idx, pollfds_.end() - 1);
        if (channelAtEnd < 0)
            channelAtEnd = -channelAtEnd - 1;
        channels_[channelAtEnd]->set_index(idx);
        pollfds_.pop_back();
    }
}






//
// Created by qiuyudai on 2023/12/1.
//

#include "TimerQueue.h"

#include <cassert>


std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
    std::vector<Entry> expired;
    const auto sentry = std::make_pair(now, nullptr);

    const auto it = timers_.lower_bound(sentry);
    assert(it == timers_.end() || now < it->first);
    std::copy(timers_.begin(), it, std::back_inserter(expired));
    timers_.erase(timers_.begin(), it);

    return expired;
}

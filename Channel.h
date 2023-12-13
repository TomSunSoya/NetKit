//
// Created by qiuyudai on 2023/11/30.
//

#ifndef CHANNEL_H
#define CHANNEL_H

#include <functional>
#include <boost/noncopyable.hpp>
#include <atomic>

#include "Timestamp.h"

class EventLoop;

class Channel : public boost::noncopyable {
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;

    Channel(EventLoop *loop, int fd);
    ~Channel();

    [[nodiscard]] EventLoop *ownerLoop() const
    {
        return loop_;
    }
    void handleEvent(Timestamp reveiveTime) const;
    void setReadCallback(const ReadEventCallback & read_callback)
    {
        readCallback_ = read_callback;
    }
    void set_write_callback(const EventCallback& write_callback)
    {
        writeCallback_ = write_callback;
    }
    void set_error_callback(const EventCallback& error_callback)
    {
        errorCallback_ = error_callback;
    }
    void set_close_callback(const EventCallback& close_callback) {
        closeCallback_ = close_callback;
    }

    [[nodiscard]] int fd() const
    {
        return fd_;
    }
    [[nodiscard]] int events() const
    {
        return events_;
    }
    void set_revents(int revt)
    {
        revents_ = revt;
    }
    [[nodiscard]] bool isNoneEvent() const
    {
        return events_ == kNoneEvent;
    }

    void enableReading()
    {
        events_ |= kReadEvent;
        update();
    }
     void enableWriting()
     {
         events_ |= kWriteEvent;
         update();
     }
     void disableWriting()
     {
         events_ &= ~kWriteEvent;
         update();
     }
     void disableAll()
     {
         events_ = kNoneEvent;
         update();
     }

     [[nodiscard]] bool isWriting() const {
        return events_ & kWriteEvent;
    }

    [[nodiscard]] int index() const
    {
        return index_;
    }

    void set_index(const int idx)
    {
        index_ = idx;
    }

private:
    void update();

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop* loop_;
    const int fd_;
    int events_;
    int revents_;
    int index_; // used by poller
    std::atomic_bool eventHandling_;

    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback errorCallback_;
    EventCallback closeCallback_;
};



#endif //CHANNEL_H

//
// Created by qiuyudai on 2023/12/14.
//

#ifndef NETKIT_CONNECTOR_H
#define NETKIT_CONNECTOR_H

#include <boost/noncopyable.hpp>
#include <functional>
#include <memory>
#include <atomic>

#include "InetAddress.h"

class EventLoop;
class Channel;

class Connector : public boost::noncopyable, public std::enable_shared_from_this<Connector> {
public:
    using NewConnectionCallback = std::function<void(int sockfd)>;
    Connector(EventLoop* loop, const InetAddress& serverAddr);
    ~Connector();

    void setNewConnectionCallback(const NewConnectionCallback &newConnectionCallback);

    void start();
    void restart();
    void stop();

private:
    enum States { kDisconnected, kConnecting, kConnected };
    static const int kMaxRetryDelayMs = 30*1000;
    static const int kInitRetryDelayMs = 500;

    void setState(States s) { state_ = s; }
    void startInLoop();
    void stopInLoop();
    void connect();
    void connecting(int sockfd);
    void handleWrite();
    void handleError();
    void retry(int sockfd);
    int removeAndResetChannel();
    void resetChannel();

    EventLoop* loop_;
    InetAddress serverAddr_;
    std::atomic_bool connect_; // atomic
    std::atomic<States> state_;  // FIXME: use atomic variable
    std::unique_ptr<Channel> channel_;
    NewConnectionCallback newConnectionCallback_;
    int retryDelayMs_;
};


#endif //NETKIT_CONNECTOR_H

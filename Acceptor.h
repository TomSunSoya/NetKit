//
// Created by qiuyudai on 2023/12/4.
//

#ifndef NETKIT_ACCEPTOR_H
#define NETKIT_ACCEPTOR_H

#include <boost/noncopyable.hpp>
#include "Socket.h"
#include "Channel.h"
// 用于accept TCP 新连接

class EventLoop;
class InetAddress;

class Acceptor : boost::noncopyable {
public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>;

    Acceptor(EventLoop* loop, const InetAddress& listenAddr);
    void setNewConnectionCallback(const NewConnectionCallback& cb) {
        newConnectionCallback_ = cb;
    }

    [[nodiscard]] bool listening() const {
        return listening_;
    }

    void listen();

private:
    void handleRead();
    EventLoop* loop_;
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listening_;
};


#endif //NETKIT_ACCEPTOR_H

//
// Created by qiuyudai on 2023/12/7.
//

#ifndef NETKIT_TCPCONNECTION_H
#define NETKIT_TCPCONNECTION_H

#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <atomic>

#include "Socket.h"
#include "Channel.h"
#include "InetAddress.h"
#include "Callbacks.h"
#include "Buffer.h"

class EventLoop;

class TcpConnection : public boost::noncopyable, public std::enable_shared_from_this<TcpConnection> {
public:
    const std::string &getName() const;

    void setCloseCallback(const CloseCallback &closeCallback);

    // called when TcpServer accepts a new connection
    void connectEstablished();      // should be called only once
    // called when TcpServer has removed me from its map
    void connectDestroyed();        // should be called only once
    void setConnectionCallback(const ConnectionCallback &connectionCallback);

    void setMessageCallback(const MessageCallback &messageCallback);

    void send(const std::string& message);
    void shutdown();

private:
    enum StateE {
        kConnecting,
        kConnected,
        kDisconnected,
        kDisconnecting
    };

    void setState(StateE s) {
        state_ = s;
    }
    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();
    void sendInLoop(const std::string& message);
    void shutdownInLoop();

    EventLoop *loop_;
    std::string name_;
    std::atomic<StateE> state_;
    std::unique_ptr<Socket> socket_;
    std::shared_ptr<Channel> channel_;
    InetAddress localAddr;
    InetAddress peerAddr;
    ConnectionCallback connectionCallback_;
    MessageCallback  messageCallback_;
    CloseCallback closeCallback_;
    Buffer inputBuffer_;
    Buffer outputBuffer_;
};


#endif //NETKIT_TCPCONNECTION_H
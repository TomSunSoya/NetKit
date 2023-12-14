//
// Created by qiuyudai on 2023/12/7.
//

#ifndef NETKIT_TCPCONNECTION_H
#define NETKIT_TCPCONNECTION_H

#include <boost/noncopyable.hpp>
#include <atomic>
#include <memory>

#include "Socket.h"
#include "Channel.h"
#include "InetAddress.h"
#include "Callbacks.h"
#include "Buffer.h"

class EventLoop;

class TcpConnection : public boost::noncopyable, public std::enable_shared_from_this<TcpConnection> {
public:
    TcpConnection(EventLoop *pLoop, std::string basicString, int i, InetAddress address, const InetAddress address1);

    const std::string &getName() const;

    void setCloseCallback(const CloseCallback &closeCallback);

    // called when TcpServer accepts a new connection
    void connectEstablished();      // should be called only once
    // called when TcpServer has removed me from its map
    void connectDestroyed();        // should be called only once
    void setConnectionCallback(const ConnectionCallback &connectionCallback);
    void setMessageCallback(const MessageCallback &messageCallback);
    void setWriteCompleteCallback(const WriteCompleteCallback &writeCompleteCallback);
    void setHighWaterMarkCallback(const HighWaterMarkCallback &highWaterMarkCallback, size_t high);

    void send(const std::string& message);
    void shutdown();
    void setTcpNoDelay(bool on);
    void setKeepAlive(bool on);

    EventLoop *getLoop() const;

    const InetAddress &localAddress() const;

    const InetAddress &peerAddress() const;

    bool connected() const {
        return state_ == kConnected;
    }

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
    WriteCompleteCallback writeCompleteCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;
    size_t highWaterMark_{};
    Buffer inputBuffer_;
    Buffer outputBuffer_;
};


#endif //NETKIT_TCPCONNECTION_H

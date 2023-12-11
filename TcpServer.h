//
// Created by qiuyudai on 2023/12/7.
//

#ifndef NETKIT_TCPSERVER_H
#define NETKIT_TCPSERVER_H


#include "InetAddress.h"
#include "Callbacks.h"
#include "Acceptor.h"

#include <unordered_map>

class EventLoop;

class TcpServer {
public:
    TcpServer(EventLoop* loop, const InetAddress& listenAddr, std::string name = "");


    ~TcpServer();

    void start();

    void setConnectionCallback(const ConnectionCallback& cb) {
        connectionCallback_ = cb;
    }

    void setMessageCallback(const MessageCallback& cb) {
        messageCallback_ = cb;
    }

private:
    // no thread safe, but in loop
    void newConnection(int sockfd, const InetAddress& peerAddr);
    void removeConnection(const TcpConnectionPtr& conn);
    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;

    EventLoop* loop_{};
    const std::string name_;
    std::unique_ptr<Acceptor> acceptor_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    bool started_{};
    int nextConnId_{};
    ConnectionMap connections_;
};


#endif //NETKIT_TCPSERVER_H

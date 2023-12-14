//
// Created by qiuyudai on 2023/12/7.
//

#include "TcpServer.h"

#include <utility>
#include "utils/Logger.h"
#include "EventLoop.h"
#include "utils/SocketsOps.h"
#include "TcpConnection.h"

void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr) {
    loop_->assertInLoopThread();
    char buf[32];
    snprintf(buf, sizeof buf, "#%d", nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    LOG_INFO << "TcpServer::newConnection [" << name_ << "] - new connection [" << connName
            << "] from " << peerAddr.toIpPort();
    InetAddress localAddr(getLocalAddr(sockfd));
    TcpConnectionPtr conn(new TcpConnection(loop_, connName, sockfd, localAddr, peerAddr));
    connections_[connName] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback([this](auto && PH1) { removeConnection(std::forward<decltype(PH1)>(PH1)); });
}

TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr, std::string name)
    : loop_(loop),
    started_(false),
    nextConnId_(0),
    acceptor_(new Acceptor(loop, listenAddr)),
    name_(std::move(name)) {

}

TcpServer::~TcpServer() {

}

void TcpServer::start() {

}

void TcpServer::removeConnection(const TcpConnectionPtr &conn) {
    loop_->assertInLoopThread();
    LOG_INFO << "TcpServer::removeConnection [" << name_ << "] - connection " << conn->getName();
    size_t n = connections_.erase(conn->getName());
    assert(n == 1);
    (void)n;
    loop_->queueInLoop([&] mutable { conn->connectDestroyed(); });
}

void TcpServer::setWriteCompleteCallback(const WriteCompleteCallback &writeCompleteCallback) {
    writeCompleteCallback_ = writeCompleteCallback;
}

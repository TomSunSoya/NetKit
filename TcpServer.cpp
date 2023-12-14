//
// Created by qiuyudai on 2023/12/7.
//

#include "TcpServer.h"

#include <utility>
#include "utils/Logger.h"
#include "EventLoop.h"
#include "utils/SocketsOps.h"
#include "TcpConnection.h"
#include "EventLoopThreadPool.h"

void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr) {
    loop_->assertInLoopThread();
    char buf[32];
    snprintf(buf, sizeof buf, "#%d", nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    LOG_INFO << "TcpServer::newConnection [" << name_ << "] - new connection [" << connName
             << "] from " << peerAddr.toIpPort();
    InetAddress localAddr(getLocalAddr(sockfd));

    // FIX poll with zero timeout to double confirm the new connection
    EventLoop *ioLoop = threadPool_->getNextLoop();
    TcpConnectionPtr conn(new TcpConnection(loop_, connName, sockfd, localAddr, peerAddr));
    connections_[connName] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback([this](auto &&PH1) { removeConnection(std::forward<decltype(PH1)>(PH1)); });
    ioLoop->runInLoop([conn] mutable { conn->connectEstablished(); });
}

TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr, std::string name)
        : loop_(loop),
          started_(false),
          nextConnId_(0),
          acceptor_(new Acceptor(loop, listenAddr)),
          name_(std::move(name)) {

}

TcpServer::~TcpServer() = default;

void TcpServer::start() {

}

void TcpServer::removeConnection(const TcpConnectionPtr &conn) {
    // TODO: unsafe
    loop_->runInLoop([this, conn] { removeConnectionInLoop(conn); });
}

void TcpServer::setWriteCompleteCallback(const WriteCompleteCallback &writeCompleteCallback) {
    writeCompleteCallback_ = writeCompleteCallback;
}

void TcpServer::setThreadNum(int numThreads) {

}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn) {
    loop_->assertInLoopThread();
    LOG_INFO << "TcpServer::removeConnectionInLoop [" << name_ << "] - connection " << conn->getName();
    auto n = connections_.erase(conn->getName());
    assert(n == 1);
    (void) n;
    auto ioLoop = conn->getLoop();
    ioLoop->queueInLoop([conn] mutable { conn->connectDestroyed(); });
}

//
// Created by qiuyudai on 2023/12/4.
//

#include "Acceptor.h"
#include "utils/SocketsOps.h"
#include "InetAddress.h"
#include "EventLoop.h"

Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenAddr) :
    loop_(loop),
    acceptSocket_(::createNonblockingOrDie(listenAddr.family())),
    acceptChannel_(loop, acceptSocket_.fd()),
    listening_(false) {
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.bindAddress(listenAddr);
    acceptChannel_.setReadCallback([this] { handleRead(); });
}

void Acceptor::listen() {
    loop_->assertInLoopThread();
    listening_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();
}

void Acceptor::handleRead() {
    loop_->assertInLoopThread();
    InetAddress peerAddr(0);
    int connfd = acceptSocket_.accept(&peerAddr);
    if (connfd >= 0) {
        if (newConnectionCallback_) {
            // todo
            Socket socket1(acceptSocket_.accept(&peerAddr));
            newConnectionCallback_(connfd, peerAddr);
        } else
            ::close(connfd);
    }
}

//
// Created by qiuyudai on 2023/12/7.
//

#include "TcpConnection.h"
#include "utils/Logger.h"
#include "EventLoop.h"
#include "utils/SocketsOps.h"

void TcpConnection::handleRead() {
    char buf[65536];
    auto n = ::read(channel_->fd(), buf, sizeof buf);
    if (n > 0) {
        auto t = shared_from_this();
        auto T = t.get();
        t.reset();
        TcpConnectionPtr temp(T);
        messageCallback_(temp, buf, n);
    } else if (n == 0)
        handleClose();
    else
        handleError();
}

const std::string &TcpConnection::getName() const {
    return name_;
}

void TcpConnection::setCloseCallback(const CloseCallback &closeCallback) {
    closeCallback_ = closeCallback;
}

void TcpConnection::handleClose() {
    loop_->assertInLoopThread();
    LOG_TRACE << "TcpConnection::handleClose state = " << state_;
    assert(state_ == kConnected);
    channel_->disableAll();
    auto t = shared_from_this();
    auto T = t.get();
    t.reset();
    TcpConnectionPtr temp(T);
    closeCallback_(temp);
}

void TcpConnection::handleError() {
    int err = getSocketError(channel_->fd());
    LOG_ERROR << "TcpConnection::handleError [" << name_ << "] - SO_ERROR = " << err << " " << strerror_tl(err);
}

void TcpConnection::connectDestroyed() {
    loop_->assertInLoopThread();
    assert(state_ == kConnected);
    setState(kDisconnected);
    channel_->disableAll();
    auto t = shared_from_this();
    auto T = t.get();
    t.reset();
    TcpConnectionPtr temp(T);
    connectionCallback_(temp);

    loop_->removeChannel(get_pointer(channel_));
}

void TcpConnection::setConnectionCallback(const ConnectionCallback &connectionCallback) {
    connectionCallback_ = connectionCallback;
}

void TcpConnection::setMessageCallback(const MessageCallback &messageCallback) {
    messageCallback_ = messageCallback;
}

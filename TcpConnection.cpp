//
// Created by qiuyudai on 2023/12/7.
//

#include "TcpConnection.h"
#include "utils/Logger.h"
#include "EventLoop.h"
#include "utils/SocketsOps.h"

void TcpConnection::handleRead(Timestamp receiveTime) {
    int saveErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &saveErrno);
    if (n > 0)
        messageCallback_(getShared(), &inputBuffer_, receiveTime);
    else if (n == 0)
        handleClose();
    else {
        errno = saveErrno;
        LOG_ERROR << "TcpConnection::handleRead";
        handleError();
    }
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
    auto temp = getShared();
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

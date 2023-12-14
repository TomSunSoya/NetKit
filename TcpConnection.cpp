//
// Created by qiuyudai on 2023/12/7.
//

#include "TcpConnection.h"
#include "utils/Logger.h"
#include "EventLoop.h"
#include "utils/SocketsOps.h"

#include <cerrno>
#include <cassert>
#include <unistd.h>

void TcpConnection::handleRead(Timestamp receiveTime) {
    int saveErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &saveErrno);
    if (n > 0)
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
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
    assert(state_ == kDisconnecting);
    channel_->disableAll();
    closeCallback_(shared_from_this());
}

void TcpConnection::handleError() {
    int err = getSocketError(channel_->fd());
    LOG_ERROR << "TcpConnection::handleError [" << name_ << "] - SO_ERROR = " << err << " " << strerror_tl(err);
}

void TcpConnection::connectDestroyed() {
    loop_->assertInLoopThread();
    assert(state_ == kDisconnecting);
    channel_->disableAll();
    connectionCallback_(shared_from_this());

    loop_->removeChannel(channel_.get());
    setState(kDisconnected);
}

void TcpConnection::setConnectionCallback(const ConnectionCallback &connectionCallback) {
    connectionCallback_ = connectionCallback;
}

void TcpConnection::setMessageCallback(const MessageCallback &messageCallback) {
    messageCallback_ = messageCallback;
}

void TcpConnection::shutdown() {
    auto expected = kConnected;
    if (state_.compare_exchange_strong(expected, kDisconnecting)) {
        TcpConnectionPtr self(shared_from_this());
        loop_->runInLoop([self] { self->shutdownInLoop(); });
    }
}

void TcpConnection::shutdownInLoop() {
    loop_->assertInLoopThread();
    if (!channel_->isWriting())
        socket_->shutdownWrite();
}

void TcpConnection::send(const std::string &message) {
    if (state_ == kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(message);
        } else {
            loop_->runInLoop([this, message] { sendInLoop(message); });
        }
    }

}

void TcpConnection::sendInLoop(const std::string &message) {
    loop_->assertInLoopThread();
    ssize_t nwrote = 0;
    // if nothing in output queue, try writing directly
    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
        nwrote = ::write(channel_->fd(), message.data(), message.size());
        if (nwrote >= 0) {
            if (static_cast<size_t>(nwrote) < message.size())
                LOG_TRACE << "I am going to write more data.";
            else if (writeCompleteCallback_)
                loop_->queueInLoop([this] { writeCompleteCallback_(shared_from_this()); });
        } else {
            nwrote = 0;
            if (errno == EWOULDBLOCK)
                LOG_ERROR << "TcpConnection::sendInLoop";
        }
    }

    assert(nwrote >= 0);
    if (static_cast<size_t>(nwrote) < message.size()) {
        outputBuffer_.append(message.data() + nwrote, message.size() - nwrote);
        if (!channel_->isWriting())
            channel_->enableWriting();
    }
}

void TcpConnection::handleWrite() {
    loop_->assertInLoopThread();
    if (channel_->isWriting()) {
        auto n = ::write(channel_->fd(), outputBuffer_.peek(), outputBuffer_.readableBytes());
        if (n > 0) {
            outputBuffer_.retrieve(n);
            if (outputBuffer_.readableBytes() == 0) {
                channel_->disableWriting();
                if (writeCompleteCallback_)
                    loop_->queueInLoop([this] { writeCompleteCallback_(shared_from_this()); });
                if (state_ == kDisconnecting)
                    shutdownInLoop();
            } else
                LOG_TRACE << "I am going to write more data";
        } else
            LOG_ERROR << "TcpConnection::handleWrite";
    } else
        LOG_TRACE << "Connection is down, no more writing";
}

void TcpConnection::setTcpNoDelay(bool on) {
    socket_->setTcpNoDelay(on);
}

void TcpConnection::setKeepAlive(bool on) {
    socket_->setKeepAlive(on);
}

void TcpConnection::setWriteCompleteCallback(const WriteCompleteCallback &writeCompleteCallback) {
    writeCompleteCallback_ = writeCompleteCallback;
}

void TcpConnection::setHighWaterMarkCallback(const HighWaterMarkCallback &highWaterMarkCallback, size_t high) {
    highWaterMarkCallback_ = highWaterMarkCallback;
    highWaterMark_ = high;
}

TcpConnection::TcpConnection(EventLoop *pLoop, std::string basicString, int i, InetAddress address,
                             const InetAddress address1) : loop_(pLoop), name_(basicString), socket_(new Socket(i)),
                                                           localAddr(address), peerAddr(address1) {

}

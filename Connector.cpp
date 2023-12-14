//
// Created by qiuyudai on 2023/12/14.
//

#include "Connector.h"
#include "Channel.h"
#include "EventLoop.h"
#include "utils/Logger.h"
#include "utils/SocketsOps.h"

#include <cassert>
#include <memory>

const int Connector::kMaxRetryDelayMs;

void Connector::setNewConnectionCallback(const Connector::NewConnectionCallback &newConnectionCallback) {
    newConnectionCallback_ = newConnectionCallback;
}

void Connector::startInLoop() {
    loop_->assertInLoopThread();
    assert(state_ == kDisconnected);
    if (connect_) {
        connect();
    } else {
        LOG_DEBUG << "do not connect";
    }
}

Connector::Connector(EventLoop *loop, const InetAddress &serverAddr)
        : loop_(loop), serverAddr_(serverAddr), connect_(false), state_(kDisconnected),
          retryDelayMs_(kInitRetryDelayMs) {
    LOG_DEBUG << "ctor[" << this << "]";
}

Connector::~Connector() {
    LOG_DEBUG << "dtor[" << this << "]";
    assert(!channel_);
}

void Connector::start() {
    connect_ = true;
    loop_->runInLoop([this] { startInLoop(); }); // FIXME: unsafe
}

void Connector::restart() {
    loop_->assertInLoopThread();
    setState(kDisconnected);
    retryDelayMs_ = kInitRetryDelayMs;
    connect_ = true;
    startInLoop();
}

void Connector::stop() {
    connect_ = false;
    loop_->queueInLoop([this] { stopInLoop(); }); // FIXME: unsafe
    // TODO: cancel timer
}

void Connector::stopInLoop() {
    loop_->assertInLoopThread();
    if (state_ == kConnecting) {
        setState(kDisconnected);
        int sockfd = removeAndResetChannel();
        retry(sockfd);
    }
}

void Connector::connect() {
    int sockfd = createNonblockingOrDie(serverAddr_.family());
    int ret = ::connect(sockfd, serverAddr_.getSockAddr());
    int savedErrno = (ret == 0) ? 0 : errno;
    switch (savedErrno) {
        case 0:
        case EINPROGRESS:
        case EINTR:
        case EISCONN:
            connecting(sockfd);
            break;

        case EAGAIN:
        case EADDRINUSE:
        case EADDRNOTAVAIL:
        case ECONNREFUSED:
        case ENETUNREACH:
            retry(sockfd);
            break;

        case EACCES:
        case EPERM:
        case EAFNOSUPPORT:
        case EALREADY:
        case EBADF:
        case EFAULT:
        case ENOTSOCK:
            LOG_ERROR << "connect error in Connector::startInLoop " << savedErrno;
            close(sockfd);
            break;

        default:
            LOG_ERROR << "Unexpected error in Connector::startInLoop " << savedErrno;
            close(sockfd);
            // connectErrorCallback_();
            break;
    }
}

void Connector::connecting(int sockfd) {
    setState(kConnecting);
    assert(!channel_);
    channel_ = std::make_unique<Channel>(loop_, sockfd);
    channel_->setWriteCallback(
            [this] { handleWrite(); }); // FIXME: unsafe
    channel_->setErrorCallback(
            [this] { handleError(); }); // FIXME: unsafe

    channel_->enableWriting();
}


void Connector::handleWrite() {
    LOG_TRACE << "Connector::handleWrite " << state_;

    if (state_ == kConnecting) {
        int sockfd = removeAndResetChannel();
        int err = getSocketError(sockfd);
        if (err) {
            LOG_WARN << "Connector::handleWrite - SO_ERROR = "
                     << err << " " << strerror_tl(err);
            retry(sockfd);
        } else if (isSelfConnect(sockfd)) {
            LOG_WARN << "Connector::handleWrite - Self connect";
            retry(sockfd);
        } else {
            setState(kConnected);
            if (connect_) {
                newConnectionCallback_(sockfd);
            } else {
                close(sockfd);
            }
        }
    } else {
        // what happened?
        assert(state_ == kDisconnected);
    }
}

void Connector::handleError() {
    LOG_ERROR << "Connector::handleError state=" << state_;
    if (state_ == kConnecting) {
        int sockfd = removeAndResetChannel();
        int err = getSocketError(sockfd);
        LOG_TRACE << "SO_ERROR = " << err << " " << strerror_tl(err);
        retry(sockfd);
    }
}

void Connector::retry(int sockfd) {
    close(sockfd);
    setState(kDisconnected);
    if (connect_) {
        LOG_INFO << "Connector::retry - Retry connecting to " << serverAddr_.toIpPort()
                 << " in " << retryDelayMs_ << " milliseconds. ";
        loop_->runAfter(retryDelayMs_ / 1000.0,
                        [capture0 = shared_from_this()] { capture0->startInLoop(); });
        retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);
    } else {
        LOG_DEBUG << "do not connect";
    }
}

int Connector::removeAndResetChannel() {
    channel_->disableAll();
    channel_->remove();
    int sockfd = channel_->fd();
    // Can't reset channel_ here, because we are inside Channel::handleEvent
    loop_->queueInLoop([this] { resetChannel(); }); // FIXME: unsafe
    return sockfd;
}

void Connector::resetChannel() {
    channel_.reset();
}
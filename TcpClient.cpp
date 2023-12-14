#include "TcpClient.h"

#include "utils/Logger.h"
#include "Connector.h"
#include "EventLoop.h"
#include "utils/SocketsOps.h"

#include <cstdio>  // snprintf
#include <utility>

using namespace std::placeholders;

void defaultConnectionCallback(const TcpConnectionPtr &conn) {
    LOG_TRACE << conn->localAddress().toIpPort() << " -> "
              << conn->peerAddress().toIpPort() << " is "
              << (conn->connected() ? "UP" : "DOWN");
    // do not call conn->forceClose(), because some users want to register message callback only.
}

void defaultMessageCallback(const TcpConnectionPtr &,
                            Buffer *buf,
                            Timestamp) {
    buf->retrieveAll();
}

namespace detail {
    void removeConnection(EventLoop *loop, const TcpConnectionPtr &conn) {
        loop->queueInLoop([conn] mutable { conn->connectDestroyed(); });
    }

    void removeConnector(const ConnectorPtr &connector) {
        //connector->
    }
};

TcpClient::TcpClient(EventLoop *loop,
                     const InetAddress &serverAddr,
                     std::string nameArg)
        : loop_(loop),
          connector_(new Connector(loop, serverAddr)),
          name_(std::move(nameArg)),
          connectionCallback_(defaultConnectionCallback),
          messageCallback_(defaultMessageCallback),
          retry_(false),
          connect_(true),
          nextConnId_(1) {
    connector_->setNewConnectionCallback(
            [this](auto && PH1) { newConnection(std::forward<decltype(PH1)>(PH1)); });
    // FIXME setConnectFailedCallback
    LOG_INFO << "TcpClient::TcpClient[" << name_
             << "] - connector " << connector_.get();
}

TcpClient::~TcpClient() {
    LOG_INFO << "TcpClient::~TcpClient[" << name_
             << "] - connector " << connector_.get();
    TcpConnectionPtr conn;
    bool unique = false;
    {
        std::scoped_lock<std::mutex> lock(mutex_);
        unique = connection_.unique();
        conn = connection_;
    }
    if (conn) {
        assert(loop_ == conn->getLoop());
        // FIXME: not 100% safe, if we are in different thread
        CloseCallback cb = [](auto && PH1) { detail::removeConnection(std::forward<decltype(PH1)>(PH1)); };
        loop_->runInLoop(
                [conn, cb] mutable { conn->setCloseCallback(cb); });
        if (unique) {
            conn->forceClose();
        }
    } else {
        connector_->stop();
        // FIXME: HACK
        loop_->runAfter(1, [this] { detail::removeConnector(connector_); });
    }
}

void TcpClient::connect() {
    // FIXME: check state
    LOG_INFO << "TcpClient::connect[" << name_ << "] - connecting to "
             << connector_->serverAddress().toIpPort();
    connect_ = true;
    connector_->start();
}

void TcpClient::disconnect() {
    connect_ = false;

    {
        std::scoped_lock lock(mutex_);
        if (connection_) {
            connection_->shutdown();
        }
    }
}

void TcpClient::stop() {
    connect_ = false;
    connector_->stop();
}

void TcpClient::newConnection(int sockfd) {
    loop_->assertInLoopThread();
    InetAddress peerAddr(getPeerAddr(sockfd));
    char buf[32];
    snprintf(buf, sizeof buf, ":%s#%d", peerAddr.toIpPort().c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    InetAddress localAddr(getLocalAddr(sockfd));
    // FIXME poll with zero timeout to double confirm the new connection
    // FIXME use make_shared if necessary
    TcpConnectionPtr conn(new TcpConnection(loop_,
                                            connName,
                                            sockfd,
                                            localAddr,
                                            peerAddr));

    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(
            [this](auto && PH1) { removeConnection(std::forward<decltype(PH1)>(PH1)); }); // FIXME: unsafe
    {
        std::scoped_lock lock(mutex_);
        connection_ = conn;
    }
    conn->connectEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr &conn) {
    loop_->assertInLoopThread();
    assert(loop_ == conn->getLoop());

    {
        std::scoped_lock lock(mutex_);
        assert(connection_ == conn);
        connection_.reset();
    }

    loop_->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    if (retry_ && connect_) {
        LOG_INFO << "TcpClient::connect[" << name_ << "] - Reconnecting to "
                 << connector_->serverAddress().toIpPort();
        connector_->restart();
    }
}


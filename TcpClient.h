//
// Created by qiuyudai on 2023/12/14.
//

#ifndef NETKIT_TCPCLIENT_H
#define NETKIT_TCPCLIENT_H

#include "TcpConnection.h"

#include <mutex>
#include <boost/noncopyable.hpp>


class Connector;

typedef std::shared_ptr<Connector> ConnectorPtr;

class TcpClient : public boost::noncopyable {
public:
    // TcpClient(EventLoop* loop);
    // TcpClient(EventLoop* loop, const string& host, uint16_t port);
    TcpClient(EventLoop *loop,
              const InetAddress &serverAddr,
              std::string nameArg);

    ~TcpClient();  // force out-line dtor, for std::unique_ptr members.

    void connect();

    void disconnect();

    void stop();

    [[nodiscard]] TcpConnectionPtr connection() const {
        std::scoped_lock<std::mutex> lock(mutex_);
        return connection_;
    }

    [[nodiscard]] EventLoop *getLoop() const { return loop_; }

    [[nodiscard]] bool retry() const { return retry_; }

    void enableRetry() { retry_ = true; }

    [[nodiscard]] const std::string &name() const { return name_; }

    /// Set connection callback.
    /// Not thread safe.
    void setConnectionCallback(ConnectionCallback cb) { connectionCallback_ = std::move(cb); }

    /// Set message callback.
    /// Not thread safe.
    void setMessageCallback(MessageCallback cb) { messageCallback_ = std::move(cb); }

    /// Set write complete callback.
    /// Not thread safe.
    void setWriteCompleteCallback(WriteCompleteCallback cb) { writeCompleteCallback_ = std::move(cb); }

private:
    /// Not thread safe, but in loop
    void newConnection(int sockfd);

    /// Not thread safe, but in loop
    void removeConnection(const TcpConnectionPtr &conn);

    EventLoop *loop_;
    ConnectorPtr connector_; // avoid revealing Connector
    const std::string name_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    bool retry_;   // atomic
    bool connect_; // atomic
    // always in loop thread
    int nextConnId_;
    mutable std::mutex mutex_;
    TcpConnectionPtr connection_;
};

#endif

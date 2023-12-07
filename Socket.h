//
// Created by qiuyudai on 2023/12/6.
// This is an internal header file, you should not include this.

#ifndef NETKIT_SOCKET_H
#define NETKIT_SOCKET_H

#include <boost/noncopyable.hpp>
#include <netinet/tcp.h>

// struct tcp_info is in <netinet/tcp.h> for linux
struct tcp_info;

class InetAddress;

class Socket : public boost::noncopyable {
public:
    explicit Socket(int sockfd) : sockfd_(sockfd) {}
    ~Socket();

    [[nodiscard]] int fd() const { return sockfd_; }
    // return true if success.
    bool getTcpInfo(struct tcp_info*) const;
    bool getTcpInfoString(char* buf, int len) const;
    /// abort if address in use
    void bindAddress(const InetAddress& localaddr) const;
    /// abort if address in use
    void listen() const;
    /// On success, returns a non-negative integer that is
    /// a descriptor for the accepted socket, which has been
    /// set to non-blocking and close-on-exec. *peeraddr is assigned.
    /// On error, -1 is returned, and *peeraddr is untouched.
    int accept(InetAddress* peeraddr) const;

    void shutdownWrite() const;

    ///
    /// Enable/disable TCP_NODELAY (disable/enable Nagle's algorithm).
    ///
    void setTcpNoDelay(bool on) const;

    ///
    /// Enable/disable SO_REUSEADDR
    ///
    void setReuseAddr(bool on) const;

    ///
    /// Enable/disable SO_REUSEPORT
    ///
    void setReusePort(bool on) const;

    ///
    /// Enable/disable SO_KEEPALIVE
    ///
    void setKeepAlive(bool on) const;
private:
    const int sockfd_;
};


#endif //NETKIT_SOCKET_H

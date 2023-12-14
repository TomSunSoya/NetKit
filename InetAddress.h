//
// Created by qiuyudai on 2023/12/6.
//

#ifndef NETKIT_INETADDRESS_H
#define NETKIT_INETADDRESS_H

#include "copyable.h"

#include <string>

#include <netinet/in.h>

const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr);

class InetAddress : public copyable {
public:
    /// Constructs an endpoint with given port number.
    /// Mostly used in TcpServer listening.
    explicit InetAddress(uint16_t port = 0, bool loopbackOnly = false, bool ipv6 = false);
    InetAddress(std::string& ip, uint16_t port, bool ipv6 = false);
    explicit InetAddress(const struct sockaddr_in& addr)
            : addr_(addr) {}
    explicit InetAddress(const struct sockaddr_in6& addr)
            : addr6_(addr) {}
    [[nodiscard]] sa_family_t family() const { return addr_.sin_family; }
    [[nodiscard]] std::string toIp() const;
    [[nodiscard]] std::string toIpPort() const;
    [[nodiscard]] uint16_t port() const;
    [[nodiscard]] const struct sockaddr* getSockAddr() const { return sockaddr_cast(&addr6_); }
    void setSockAddrInet6(const struct sockaddr_in6& addr6) { addr6_ = addr6; }

    [[nodiscard]] uint32_t ipv4NetEndian() const;
    [[nodiscard]] uint16_t portNetEndian() const { return addr_.sin_port; }

    // resolve hostname to IP address, not changing port or sin_family
    // return true on success.
    // thread safe
    static bool resolve(std::string& hostname, InetAddress* result);
    // static std::vector<InetAddress> resolveAll(const char* hostname, uint16_t port = 0);

    // set IPv6 ScopeID
    void setScopeId(uint32_t scope_id);

    basic_ostream <_Ch, _Tr> &toHostPort();

private:
    union {
        struct sockaddr_in addr_;
        struct sockaddr_in6 addr6_;
    };
};


#endif //NETKIT_INETADDRESS_H

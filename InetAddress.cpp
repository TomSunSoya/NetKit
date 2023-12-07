//
// Created by qiuyudai on 2023/12/6.
//

#include "InetAddress.h"
#include "SocketsOps.h"
#include "Endian.h"
#include "Logger.h"

#include <cstring>
#include <cassert>

#include <netdb.h>
#include <netinet/in.h>

// INADDR_ANY use (type)value casting.
#pragma GCC diagnostic ignored "-Wold-style-cast"
static const in_addr_t kInaddrAny = INADDR_ANY;
static const in_addr_t kInaddrLoopback = INADDR_LOOPBACK;
#pragma GCC diagnostic error "-Wold-style-cast"

static_assert(sizeof(InetAddress) == sizeof(struct sockaddr_in6),
              "InetAddress is same size as sockaddr_in6");

// todo 偏移应当是0
static_assert(offsetof(sockaddr_in, sin_family) == 1, "sin_family offset 0");
static_assert(offsetof(sockaddr_in6, sin6_family) == 1, "sin6_family offset 0");
static_assert(offsetof(sockaddr_in, sin_port) == 2, "sin_port offset 2");
static_assert(offsetof(sockaddr_in6, sin6_port) == 2, "sin6_port offset 2");

InetAddress::InetAddress(uint16_t portArg, bool loopbackOnly, bool ipv6) {
    static_assert(offsetof(InetAddress, addr6_) == 0, "addr6_ offset 0");
    static_assert(offsetof(InetAddress, addr_) == 0, "addr_ offset 0");
    if (ipv6) {
        bzero(&addr6_, sizeof addr6_);
        addr6_.sin6_family = AF_INET6;
        in6_addr ip = loopbackOnly ? in6addr_loopback : in6addr_any;
        addr6_.sin6_addr = ip;
        addr6_.sin6_port = hostToNetwork16(portArg);
    } else {
        bzero(&addr_, sizeof addr_);
        addr_.sin_family = AF_INET;
        in_addr_t ip = loopbackOnly ? kInaddrLoopback : kInaddrAny;
        addr_.sin_addr.s_addr = hostToNetwork32(ip);
        addr_.sin_port = hostToNetwork16(portArg);
    }
}

InetAddress::InetAddress(std::string &ip, uint16_t portArg, bool ipv6) {
    if (ipv6 || strchr(ip.c_str(), ':')) {
        bzero(&addr6_, sizeof addr6_);
        fromIpPort(ip.c_str(), portArg, &addr6_);
    } else {
        bzero(&addr_, sizeof addr_);
        fromIpPort(ip.c_str(), portArg, &addr_);
    }
}

std::string InetAddress::toIp() const {
    char buf[64] = "";
    ::toIpPort(buf, sizeof buf, getSockAddr());
    return buf;
}

std::string InetAddress::toIpPort() const {
    char buf[64] = "";
    ::toIp(buf, sizeof buf, getSockAddr());
    return buf;
}

uint16_t InetAddress::port() const {
    return networkToHost16(portNetEndian());
}


uint32_t InetAddress::ipv4NetEndian() const {
    assert(family() == AF_INET);
    return addr_.sin_addr.s_addr;
}

static thread_local char t_resolveBuffer[64 * 1024];

bool InetAddress::resolve(std::string &hostname, InetAddress *out) {
    assert(out != nullptr);
    struct addrinfo hints{};
    struct addrinfo *result = nullptr;

    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // 只考虑 IPv4 地址
    hints.ai_socktype = SOCK_STREAM; // 流式套接字
    hints.ai_flags = AI_CANONNAME;

    int ret = getaddrinfo(hostname.c_str(), nullptr, &hints, &result);
    if (ret != 0) {
        LOG_ERROR << "InetAddress::resolve";
        return false;
    }
    if (result) {
        // 假设只获取第一个地址
        auto *host_addr = reinterpret_cast<struct sockaddr_in *>(result->ai_addr);
        out->addr_ = *host_addr;
        freeaddrinfo(result);
        return true;
    }
    return false;
}

void InetAddress::setScopeId(uint32_t scope_id) {
    if (family() == AF_INET6) {
        addr6_.sin6_scope_id = scope_id;
    }
}

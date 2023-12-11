//
// Created by qiuyudai on 2023/12/9.
//

#include "../EventLoop.h"
#include "../utils/Logger.h"
#include "../TcpConnection.h"
#include "../TcpServer.h"

void onConnection(const TcpConnectionPtr& conn) {
    if (conn->connected()) {
        LOG_INFO << "onConnection(): new connection [" << conn->getName() << "] from " << conn->peerAddress().toHostPort() << '\n';
    } else {
        LOG_INFO << "onConnection(): connection [" << conn->getName() << "] is down\n";
    }
}

void onMessage(const TcpConnectionPtr& conn, const char* data, ssize_t len) {
    LOG_INFO << "onMessage(): received " << len << " bytes from connection [" << conn->getName() << '\n';
}

int main() {
    LOG_INFO << "main(): pid = " << getpid() << '\n';

    InetAddress listenAddr(9981);
    EventLoop loop;

    TcpServer server(&loop, listenAddr);
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);
    server.start();

    loop.loop();
}
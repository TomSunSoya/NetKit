//
// Created by qiuyudai on 2023/12/7.
//
#include "../EventLoop.h"
#include "../InetAddress.h"
#include "../Acceptor.h"
#include "../Logger.h"


void newConnection(int sockfd, const InetAddress& peerAddr) {
    LOG_INFO << "newConnection(): accepted a new connnection from " << peerAddr.toIpPort() << '\n';
    ::write(sockfd, "How are you!\n", 13);
    ::close(sockfd);
}

int main() {
    LOG_INFO << "main(): pid = " << getpid();

    InetAddress listenAddr(9981);
    EventLoop loop;
    Acceptor acceptor(&loop, listenAddr);
    acceptor.setNewConnectionCallback(newConnection);
    acceptor.listen();

    loop.loop();
}
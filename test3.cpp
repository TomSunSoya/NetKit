//
// Created by qiuyudai on 2023/11/30.
//
#include <chrono>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>

#include "EventLoop.h"
#include "Channel.h"

EventLoop* g_loop;

class TimerFD {
public:
    TimerFD() {
        if (pipe(pipe_fds) == -1) {
            throw std::runtime_error("Failed to create pipe");
        }

        // 设置读取端为非阻塞模式
        int flags = fcntl(pipe_fds[0], F_GETFL, 0);
        fcntl(pipe_fds[0], F_SETFL, flags | O_NONBLOCK);
    }

    ~TimerFD() {
        close(pipe_fds[0]);
        close(pipe_fds[1]);
        if (timer_thread.joinable()) {
            timer_thread.join();
        }
    }

    void start_timer(int seconds) {
        timer_thread = std::thread([this, seconds]() {
            std::this_thread::sleep_for(std::chrono::seconds(seconds));
            write(pipe_fds[1], "timer expired", 13);
        });
    }

    [[nodiscard]] int get_fd() const {
        return pipe_fds[0]; // 返回读取端的文件描述符
    }

private:
    int pipe_fds[2]{}; // 管道的文件描述符
    std::thread timer_thread;
};

int main()
{
    EventLoop loop;
    g_loop = &loop;

    TimerFD timer_fd;
    int timefd = timer_fd.get_fd();
    Channel channel(&loop, timefd);
    channel.setReadCallback([]
    {
        std::cout << "Timeout!\n";
        g_loop->quit();
    });
    channel.enableReading();

    struct itimerspec howlong;
}
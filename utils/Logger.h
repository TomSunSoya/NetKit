//
// Created by qiuyudai on 2023/12/6.
//

#ifndef NETKIT_LOGGER_H
#define NETKIT_LOGGER_H

#include <iostream>
#include <sstream>
#include <string>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <thread>
#include <cstring>
#include <ctime>

enum class LogLevel {
    TRACE,
    INFO,
    ERROR,
    FATAL,
    WARN
};

class Logger {
public:
    explicit Logger(LogLevel level) : logLevel(level) {}

    template<typename T>
    Logger& operator<<(const T& message) {
        std::ostringstream stream;
        stream << message;
        buffer += stream.str();
        return *this;
    }

    ~Logger() {
        std::lock_guard<std::mutex> guard(mutex_);
        std::cout << "[" << getCurrentTime() << "] "
                  << "[" << logLevelToString(logLevel) << "] "
                  << buffer << std::endl;
    }

private:
    std::string buffer;
    LogLevel logLevel;
    static std::mutex mutex_;

    static std::string logLevelToString(LogLevel level) {
        switch (level) {
            case LogLevel::TRACE: return "TRACE";
            case LogLevel::INFO: return "INFO";
            case LogLevel::ERROR: return "ERROR";
            case LogLevel::FATAL: return "FATAL";
            case LogLevel::WARN: return "WARN";
            default: return "UNKNOWN";
        }
    }

    static std::string getCurrentTime() {
        using namespace std::chrono;
        auto now = system_clock::now();
        auto now_ms = time_point_cast<milliseconds>(now);
        auto epoch = now_ms.time_since_epoch();
        auto value = duration_cast<milliseconds>(epoch);
        std::time_t t = system_clock::to_time_t(now);
        std::tm tm{};
        localtime_r(&t, &tm); // Thread-safe localtime
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        oss << '.' << std::setfill('0') << std::setw(3) << value.count() % 1000;
        return oss.str();
    }
};

std::mutex Logger::mutex_;

// 宏定义
#define LOG_TRACE Logger(LogLevel::TRACE)
#define LOG_INFO Logger(LogLevel::INFO)
#define LOG_ERROR Logger(LogLevel::ERROR)
#define LOG_FATAL Logger(LogLevel::FATAL)
#define LOG_WARN Logger(LogLevel::WARN)

thread_local char t_errnobuf[512];

const char* strerror_tl(int savedErrno)
{
    return strerror_r(savedErrno, t_errnobuf, sizeof t_errnobuf);
}


#endif //NETKIT_LOGGER_H

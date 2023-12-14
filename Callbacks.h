//
// Created by qiuyudai on 2023/12/1.
//

#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <functional>
#include <memory>
#include "Timestamp.h"

class Buffer;
class TcpConnection;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
using CloseCallback = std::function<void(const TcpConnectionPtr&)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr&)>;
using HighWaterMarkCallback = std::function<void(const TcpConnectionPtr&, size_t)>;
using MessageCallback = std::function<void (const TcpConnectionPtr&, Buffer*, Timestamp)>;
using TimerCallback = std::function<void()>;
using Functor = std::function<void()>;


#endif //CALLBACKS_H

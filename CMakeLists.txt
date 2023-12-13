cmake_minimum_required(VERSION 3.0)
project(NetKit)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include_directories(/usr/include)
include_directories(/opt/homebrew/Cellar/boost/1.83.0/include)
# include_directories(/opt/homebrew/Cellar/spd log/1.12.0/include/)
# add_compile_options(-fmodules-ts -fmodule-file=/Users/dqy/CLionProjects/Tour/gcm.cache/opt/homebrew/Cellar/gcc/13.2.0/include/c++/13/iostream.gcm)
# file(GLOB source_files "base/*.cc" "base/*.h")
# file(GLOB Boost_INCLUDE_DIRS "/opt/homebrew/Cellar/boost/1.83.0/include")
# file(GLOB Boost_LIBRARIES "/opt/homebrew/Cellar/boost/1.83.0/lib")
add_compile_options(-g)
# find_package(spdlog REQUIRED)

# if(Boost_FOUND)
# include_directories(${Boost_INCLUDE_DIRS})
add_executable(NetKit
        EventLoop.cpp
        EventLoop.h
        # test1.cpp
        # test2.cpp
        Channel.cpp
        Channel.h
        Poller.cpp
        Poller.h
        Timestamp.h
        Timestamp.cpp
		#test/test3.cpp
        Timer.cpp
        Timer.h
        TimerId.h
        TimerQueue.cpp
        TimerQueue.h
        Callbacks.h
		#test/test4.cpp
        EventLoopThread.cpp
        EventLoopThread.h
        Acceptor.cpp
        Acceptor.h
        utils/SocketsOps.cpp
        utils/SocketsOps.h
        copyable.h
        Endian.h
        InetAddress.cpp
        InetAddress.h
        utils/Logger.h
        Socket.cpp
        Socket.h
        test/test5.cpp
        TcpServer.cpp
        TcpServer.h
        TcpConnection.cpp
        TcpConnection.h
        test/test6.cpp
        Buffer.cpp
        Buffer.h)
# target_link_libraries(NetKit PRIVATE /usr/lib)
# target_link_libraries(NetKit PRIVATE /opt/homebrew/Cellar/boost/1.83.0/lib)
# endif()




#target_link_libraries(NetKit spdlog::spdlog)

cmake_minimum_required(VERSION 3.15)
project(libevent_demo)

set(CMAKE_CXX_STANDARD 14)

add_executable(libevent_demo main.cpp epoll/epoll_server_001.cpp epoll/epoll_client_001.cpp epoll/tmp.cpp)
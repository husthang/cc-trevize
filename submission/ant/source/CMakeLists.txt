cmake_minimum_required(VERSION 3.4)
project(source)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    future_net.cpp
    future_net.h)

add_executable(source ${SOURCE_FILES})
cmake_minimum_required(VERSION 3.0)
project(deepin_installer)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp)
add_executable(deepin_installer ${SOURCE_FILES})
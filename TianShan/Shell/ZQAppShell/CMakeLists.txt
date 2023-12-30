cmake_minimum_required(VERSION 2.8)

project(ServiceShell)

include(${PROJECT_SOURCE_DIR}/../../../ZQProjs.cmake)
include_directories(${zqcommonfolder})
include_directories(${zqprojsroot})
include_directories(../../include)

add_library(ServiceShell STATIC ServiceShell.cpp)

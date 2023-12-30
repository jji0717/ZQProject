cmake_minimum_required(VERSION 2.8)

project(SystemShell)

include(${PROJECT_SOURCE_DIR}/../../../ZQProjs.cmake)
include_directories(${zqcommonfolder})
include_directories(${zqprojsroot})
include_directories(../../include)

add_executable(SystemShell SystemShell.cpp)
target_link_libraries(SystemShell ZQCommon SnmpManPkg)
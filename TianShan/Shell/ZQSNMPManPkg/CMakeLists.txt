cmake_minimum_required(VERSION 2.8)

project(SnmpManPkg)

include(${PROJECT_SOURCE_DIR}/../../../ZQProjs.cmake)
include_directories(${zqcommonfolder})
include_directories(${zqprojsroot})
include_directories(../../include)

add_library(SnmpManPkg SHARED SnmpManPkg.cpp SnmpSubagent.cpp SnmpUtil.cpp VarInfo.cpp Mib.cpp Var.cpp)
target_link_libraries(SnmpManPkg snmp)
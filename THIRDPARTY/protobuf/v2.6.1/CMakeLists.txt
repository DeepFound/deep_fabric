# CMake build script for txObject
cmake_minimum_required(VERSION 2.8.11)
project(deepis-protobuf)

set(TOP ${CMAKE_SOURCE_DIR}/../../..)

include(${TOP}/build/cmake/Common.cmake)

set(PROTOBUF_CONFIG_FILE ${REPOSITORY}/Find${PROJECT_NAME}.cmake)

file(WRITE  ${PROTOBUF_CONFIG_FILE} "set(DEEPIS_PROTOBUF_LIB ${CMAKE_SOURCE_DIR}/build/target/lib/libprotobuf.a)\n")
file(APPEND ${PROTOBUF_CONFIG_FILE} "set(DEEPIS_PROTOBUF_INC ${CMAKE_SOURCE_DIR}/build/target/include)\n")
file(APPEND ${PROTOBUF_CONFIG_FILE} "set(DEEPIS_PROTOBUF_BIN ${CMAKE_SOURCE_DIR}/build/target/bin)\n")

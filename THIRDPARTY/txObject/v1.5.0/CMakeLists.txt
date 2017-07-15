# CMake build script for txObject
cmake_minimum_required(VERSION 2.8.11)
project(deepis-txobject)

set(TOP ${CMAKE_SOURCE_DIR}/../../..)

include(${TOP}/build/cmake/Common.cmake)

set(TXOBJECT_CONFIG_FILE ${REPOSITORY}/Find${PROJECT_NAME}.cmake)

file(WRITE  ${TXOBJECT_CONFIG_FILE} "set(DEEPIS_TXOBJECT_LIB ${CMAKE_SOURCE_DIR}/build/target/lib/libtxobject.a)\n")
file(APPEND ${TXOBJECT_CONFIG_FILE} "set(DEEPIS_TXOBJECT_INC ${CMAKE_SOURCE_DIR}/build/target/include)\n")

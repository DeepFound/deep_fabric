# CMake build script for ZeroMQ

cmake_minimum_required(VERSION 2.8.11)
project(deepis-libzmq)
#
# END DEEPIS EXTENSION
#

#
# BEGIN DEEPIS EXTENSION
#
set(TOP ${CMAKE_SOURCE_DIR}/../../..)

include(${TOP}/build/cmake/Common.cmake)

set(LIBZMQ_CONFIG_FILE ${REPOSITORY}/Find${PROJECT_NAME}.cmake)

file(WRITE ${LIBZMQ_CONFIG_FILE} "set(DEEPIS_LIBZMQ_LIB ${CMAKE_SOURCE_DIR}/build/target/lib/libzmq.a)\n")
file(APPEND ${LIBZMQ_CONFIG_FILE} "set(DEEPIS_LIBZMQ_INC ${CMAKE_SOURCE_DIR}/build/target/include)\n")
#
# END DEEPIS EXTENSION
#

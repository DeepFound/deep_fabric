# CMake build script for Q
cmake_minimum_required(VERSION 2.8)
project(deepis-q)

#
# END DEEPIS EXTENSION
#

#
# BEGIN DEEPIS EXTENSION
#
set(DEEPIS_Q_EXEC_NAME deepis-q)

set(TOP ${CMAKE_SOURCE_DIR}/../..)

include(${TOP}/build/cmake/Common.cmake)

set(DEEPIS_Q_CONFIG_FILE ${REPOSITORY}/Find${PROJECT_NAME}.cmake)

file(WRITE ${DEEPIS_Q_CONFIG_FILE} "set(DEEPIS_Q_LIB ${CMAKE_SOURCE_DIR}/build/libdeepis-q.a)\n")
file(APPEND ${DEEPIS_Q_CONFIG_FILE} "set(DEEPIS_Q_INC ${CMAKE_SOURCE_DIR})\n")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -DCACHE_LINE_SIZE=${CACHE_LINE_SIZE}" )

#
# Source Files
#
set(DEEPIS_Q_SRC
  q.c
)

#
# Create library
#
set(DEEPIS_Q_LIB ${PROJECT_NAME})
add_library(${DEEPIS_Q_LIB} STATIC ${DEEPIS_Q_SRC})



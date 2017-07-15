# ----------------------------------------------------------------------------
# Copyright (C) 2015 Verizon.  All Rights Reserved.
# All Rights Reserved
#
#   Author: Donnevan Scott yeager
#   Date:   02/03/2015
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
# ------------------------------------------------------------------------------

# Project setup

# CMake build script for json_parser
cmake_minimum_required(VERSION 2.8.11)
project(deepis-json_parser)
#
# END DEEPIS EXTENSION
#

# Definitions
add_definitions(-Wall)
add_definitions(-Werror)
add_definitions(-Weffc++)

enable_testing()

# include source and test directories
add_subdirectory(tests)

#
# BEGIN DEEPIS EXTENSION
#
set(TOP ${CMAKE_SOURCE_DIR}/../..)

include(${TOP}/build/cmake/Common.cmake)

set(JSON_PARSER_CONFIG_FILE ${REPOSITORY}/Find${PROJECT_NAME}.cmake)

file(APPEND ${JSON_PARSER_CONFIG_FILE} "set(DEEPIS_JSON_PARSER_INC ${CMAKE_SOURCE_DIR}/include)\n")
#
# END DEEPIS EXTENSION
#

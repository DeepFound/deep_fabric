cmake_minimum_required(VERSION 2.8)

project(deepis-common)

set(TOP ${CMAKE_SOURCE_DIR}/..)

include(${TOP}/build/cmake/Common.cmake)

#
# Import packages
#
find_package(deepis-platform REQUIRED)
find_package(deepis-db-service REQUIRED)
find_package(deepis-db-datastore REQUIRED)

#
# Includes
#
set(DEEPIS_COMMON_INC ${CMAKE_SOURCE_DIR}/src/main/native)

include_directories(${DEEPIS_STANDARD_INCS} ${DEEPIS_PLATFORM_INC})
include_directories(${DEEPIS_STANDARD_INCS} ${DEEPIS_LIBZMQ_INC})
include_directories(${DEEPIS_STANDARD_INCS} ${DEEPIS_TXOBJECT_INC})
include_directories(${DEEPIS_STANDARD_INCS} ${DEEPIS_PROTOBUF_INC})
include_directories(${DEEPIS_STANDARD_INCS} ${DEEPIS_DB_SERVICE_INC})
include_directories(${DEEPIS_STANDARD_INCS} ${DEEPIS_DB_DATASTORE_INC})
include_directories(${DEEPIS_STANDARD_INCS} ${DEEPIS_COMMON_INC})

#
# Source Files
#
set (DEEPIS_COMMON_FABCASSI_SRC
  # common
  src/main/native/com/deepis/communication/fabricconnector/DummyBridge.cxx
  src/main/native/com/deepis/communication/fabricconnector/CassiServiceBridge.cxx
  src/main/native/com/deepis/communication/fabricconnector/FabricCassiMessageQueue.cxx
  src/main/native/com/deepis/communication/fabricconnector/FabricMessageService.cxx
  src/main/native/com/deepis/communication/fabricconnector/FabricCassiJSONAPI.cxx
  src/main/native/com/deepis/communication/fabricconnector/RealtimeFabricAPI.cxx
  src/main/native/com/deepis/communication/fabricconnector/CassiDatabase.cxx
)

#
# Create library
#
set(DEEPIS_COMMON_FABCASSI_LIB ${PROJECT_NAME}-fab-cassi)
add_library(${DEEPIS_COMMON_FABCASSI_LIB} STATIC ${DEEPIS_COMMON_FABCASSI_SRC})
target_link_libraries(${DEEPIS_COMMON_FABCASSI_LIB} ${DEEPIS_DB_DATASTORE_LIB} ${DEEPIS_PLATFORM_LIB} ${DEEPIS_DB_SERVICE_LIB})
set(DEEPIS_TEST_LIBS ${DEEPIS_COMMON_FABCASSI_LIB} ${DEEPIS_DB_DATASTORE_LIB} ${DEEPIS_PLATFORM_LIB} ${DEEPIS_STANDARD_LIBS})

#
# Export package
#
set(COMMON_CONFIG_FILE ${REPOSITORY}/Find${PROJECT_NAME}.cmake)
set(COMMON_EXPORTS_FILE ${PROJECT_BINARY_DIR}/Export${PROJECT_NAME}.cmake)

export(TARGETS ${DEEPIS_COMMON_LIB} FILE ${COMMON_EXPORTS_FILE})

file(WRITE ${COMMON_CONFIG_FILE} "set(DEEPIS_COMMON_INC ${DEEPIS_COMMON_INC})\n")
file(APPEND ${COMMON_CONFIG_FILE} "set(DEEPIS_COMMON_LIB ${CMAKE_SOURCE_DIR}/build/libdeepis-common-fab-cassi.a)\n")
file(APPEND ${COMMON_CONFIG_FILE} "include(${COMMON_EXPORTS_FILE})\n")

#
# Test
#

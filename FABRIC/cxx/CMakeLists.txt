cmake_minimum_required(VERSION 2.8)

project(deepis-fabric-cxx)
set(FABRIC_EXEC_NAME deepis-fabric)

set(TOP ${CMAKE_SOURCE_DIR}/../..)

include(${TOP}/build/cmake/Common.cmake)

# Code Profiler library: gprof
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
#set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pg")

#
# Import packages
#
find_package(deepis-platform REQUIRED)
find_package(deepis-db-service REQUIRED)
find_package(deepis-db-datastore REQUIRED)
find_package(deepis-common REQUIRED)

#
# Includes
#
set(DEEPIS_FABRIC_INC ${CMAKE_SOURCE_DIR}/src/main/native/)
include_directories(${DEEPIS_STANDARD_INCS} ${DEEPIS_FABRIC_INC})
include_directories(${DEEPIS_STANDARD_INCS} ${DEEPIS_COMMON_INC})
include_directories(${DEEPIS_STANDARD_INCS} ${DEEPIS_PLATFORM_INC})
include_directories(${DEEPIS_STANDARD_INCS} ${DEEPIS_DB_SERVICE_INC})
include_directories(${DEEPIS_STANDARD_INCS} ${DEEPIS_DB_DATASTORE_INC})
include_directories(${DEEPIS_STANDARD_INCS} ${DEEPIS_LIBZMQ_INC})
include_directories(${DEEPIS_STANDARD_INCS} ${DEEPIS_TXOBJECT_INC})
include_directories(${DEEPIS_STANDARD_INCS} ${DEEPIS_PROTOBUF_INC})

#
# Source Files
#
set (DEEPIS_FABRIC_SRC
  # fabric
  src/main/native/cxx/fabric/RealtimeFabric.cxx
  src/main/native/cxx/fabric/RealtimeFabricMatrix.cxx
)

set (DEEPIS_FABRIC_INSTANCE_SRC
  src/main/native/cxx/fabric/RealtimeFabricInstance.cxx
)

set (DEEPIS_REPLICATION_SRC
  src/main/native/cxx/replication/FileReplication.cxx
)

#
# Create library
#
set(DEEPIS_FABRIC_LIB ${PROJECT_NAME})
add_library(${DEEPIS_FABRIC_LIB} STATIC ${DEEPIS_FABRIC_SRC})


set(DEEPIS_REPLICATION_LIB ${PROJECT_NAME}-replication)
add_library(${DEEPIS_REPLICATION_LIB} STATIC ${DEEPIS_REPLICATION_SRC})

set(DEEPIS_TEST_LIBS ${DEEPIS_COMMON_FABCASSI_LIB} ${DEEPIS_DB_DATASTORE_LIB} ${DEEPIS_COMMON_LIB} ${DEEPIS_FABRIC_LIB} ${DEEPIS_REPLICATION_LIB} ${DEEPIS_STANDARD_LIBS} ${DEEPIS_PLATFORM_LIB})

#
# Export package
#
set(FABRIC_CONFIG_FILE ${REPOSITORY}/Find${PROJECT_NAME}.cmake)
set(FABRIC_EXPORTS_FILE ${PROJECT_BINARY_DIR}/Export${PROJECT_NAME}.cmake)

export(TARGETS ${DEEPIS_FABRIC_LIB} FILE ${FABRIC_EXPORTS_FILE})

file(WRITE ${FABRIC_CONFIG_FILE} "set(DEEPIS_FABRIC_INC ${DEEPIS_FABRIC_INC})\n")
file(APPEND ${FABRIC_CONFIG_FILE} "set(DEEPIS_FABRIC_LIB ${DEEPIS_FABRIC_LIB})\n")
file(APPEND ${FABRIC_CONFIG_FILE} "include(${FABRIC_EXPORTS_FILE})\n")

#
# Create Executable
#
add_executable(${FABRIC_EXEC_NAME} ${DEEPIS_FABRIC_INSTANCE_SRC})
target_link_libraries(${FABRIC_EXEC_NAME} ${DEEPIS_FABRIC_LIB})
target_link_libraries(${FABRIC_EXEC_NAME} ${DEEPIS_COMMON_LIB})
target_link_libraries(${FABRIC_EXEC_NAME} ${DEEPIS_DB_DATASTORE_LIB})
target_link_libraries(${FABRIC_EXEC_NAME} ${DEEPIS_REPLICATION_LIB})
target_link_libraries(${FABRIC_EXEC_NAME} ${DEEPIS_PLATFORM_LIB})
target_link_libraries(${FABRIC_EXEC_NAME} ${DEEPIS_STANDARD_LIBS})

#
# Test
#
#add_deep_test(TestMaster test/src/main/native/cxx/fabric/TestMaster.cxx ${DEEPIS_TEST_LIBS})
#add_deep_test(TestSlave test/src/main/native/cxx/fabric/TestSlave.cxx ${DEEPIS_TEST_LIBS})
add_deep_test(TestHASyncSlave test/src/main/native/cxx/fabric/TestHASyncSlave.cxx ${DEEPIS_TEST_LIBS})
add_deep_test(TestHAASyncSlave test/src/main/native/cxx/fabric/TestHAASyncSlave.cxx ${DEEPIS_TEST_LIBS})
add_deep_test(TestHAASync2Slaves test/src/main/native/cxx/fabric/TestHAASync2Slaves.cxx ${DEEPIS_TEST_LIBS})
add_deep_test(TestFabricService test/src/main/native/cxx/fabric/TestFabricService.cxx ${DEEPIS_TEST_LIBS})
add_deep_test(TestFabricServiceShutdown test/src/main/native/cxx/fabric/TestFabricServiceShutdown.cxx ${DEEPIS_TEST_LIBS})
add_deep_test(TestMultiSlave test/src/main/native/cxx/fabric/TestFabricServiceMultiSlave.cxx ${DEEPIS_TEST_LIBS})


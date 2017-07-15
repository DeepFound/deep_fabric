cmake_minimum_required(VERSION 2.6)

project(deepis-db-datastore)

set(TOP ${CMAKE_SOURCE_DIR}/..)

include(${TOP}/build/cmake/Common.cmake)

#
# Import packages
#
find_package(deepis-platform REQUIRED)
find_package(deepis-db-service REQUIRED)

#
# Includes
#
set(DEEPIS_DB_DATASTORE_INC ${CMAKE_SOURCE_DIR}/src/main/native)
#set(JSON_PARSE ${TOP}/THIRDPARTY/json_parser/include)

#include_directories(${DEEPIS_STANDARD_INCS} ${DEEPIS_PLATFORM_INC} ${DEEPIS_DB_SERVICE_INC} ${DEEPIS_DB_DATASTORE_INC} ${JSON_PARSE})
include_directories(${DEEPIS_STANDARD_INCS} ${DEEPIS_PLATFORM_INC} ${DEEPIS_DB_SERVICE_INC} ${DEEPIS_DB_DATASTORE_INC})

#
# Source Files
#
set (DEEPIS_DB_DATASTORE_SRC
  src/main/native/com/deepis/datastore/api/DeepStore.cxx
  )

#
# Create library
#
set(DEEPIS_DB_DATASTORE_LIB ${PROJECT_NAME})

add_library(${DEEPIS_DB_DATASTORE_LIB} STATIC ${DEEPIS_DB_DATASTORE_SRC})

target_link_libraries(${DEEPIS_DB_DATASTORE_LIB} ${DEEPIS_PLATFORM_LIB} ${DEEPIS_DB_SERVICE_LIB})

set(DEEPIS_TEST_LIBS ${DEEPIS_DB_DATASTORE_LIB} ${DEEPIS_PLATFORM_LIB} ${DEEPIS_STANDARD_LIBS})

#
# Export package
#
set(DB_DATASTORE_CONFIG_FILE ${REPOSITORY}/Find${PROJECT_NAME}.cmake)
set(DB_DATASTORE_EXPORTS_FILE ${PROJECT_BINARY_DIR}/Export${PROJECT_NAME}.cmake)

export(TARGETS ${DEEPIS_DB_DATASTORE_LIB} FILE ${DB_DATASTORE_EXPORTS_FILE})

file(WRITE ${DB_DATASTORE_CONFIG_FILE} "set(DEEPIS_DB_DATASTORE_INC ${DEEPIS_DB_DATASTORE_INC})\n")
file(APPEND ${DB_DATASTORE_CONFIG_FILE} "set(DEEPIS_DB_DATASTORE_LIB ${DEEPIS_DB_DATASTORE_LIB})\n")
file(APPEND ${DB_DATASTORE_CONFIG_FILE} "include(${DB_DATASTORE_EXPORTS_FILE})\n")

#
# Test
#
#add_deep_test(UnitTest src/test/native/com/deepis/datastore/api/TestUnit.cxx ${DEEPIS_TEST_LIBS})
#add_deep_test(ConcurrentTest src/test/native/com/deepis/datastore/api/TestConcurrent.cxx ${DEEPIS_TEST_LIBS})
#add_deep_test(CompositeTest src/test/native/com/deepis/datastore/api/TestComposite.cxx ${DEEPIS_TEST_LIBS})
#add_deep_test(DeadlockTest src/test/native/com/deepis/datastore/api/TestDeadlock.cxx ${DEEPIS_TEST_LIBS})
#add_deep_test(UniqueTest src/test/native/com/deepis/datastore/api/TestUnique.cxx ${DEEPIS_TEST_LIBS})
#add_deep_test(UpdateTest src/test/native/com/deepis/datastore/api/TestUpdate.cxx ${DEEPIS_TEST_LIBS})
#add_deep_test(TimeoutTest src/test/native/com/deepis/datastore/api/TestTimeout.cxx ${DEEPIS_TEST_LIBS})
#add_deep_test(TransactionTest src/test/native/com/deepis/datastore/api/TestTransaction.cxx ${DEEPIS_TEST_LIBS})
#add_deep_test(SchemaTest src/test/native/com/deepis/datastore/api/TestSchema.cxx ${DEEPIS_TEST_LIBS})
#add_deep_test(ReserveTest src/test/native/com/deepis/datastore/api/TestReserve.cxx ${DEEPIS_TEST_LIBS})
#add_deep_test(ACPTest src/test/native/com/deepis/datastore/api/TestACP.cxx ${DEEPIS_TEST_LIBS})
#add_deep_test(DistributedSchemaTest src/test/native/com/deepis/datastore/distributed/TestDistributedSchema.cxx ${DEEPIS_TEST_LIBS})
#add_deep_test(TestJSONParse src/test/native/com/deepis/datastore/api/TestJSONParse.cxx ${DEEPIS_TEST_LIBS})

#add_deep_test(IBenchTest src/test/native/com/deepis/datastore/api/TestIBench.cxx ${DEEPIS_TEST_LIBS})
#add_deep_test(DBT2Test src/test/native/com/deepis/datastore/api/TestDBT2.cxx ${DEEPIS_TEST_LIBS})
#add_deep_test(SecondaryKeyOnlyTest src/test/native/com/deepis/datastore/api/TestSecondaryKeyOnly.cxx ${DEEPIS_TEST_LIBS})
#add_deep_test(ValueCompressionTest src/test/native/com/deepis/datastore/api/TestValueCompression.cxx ${DEEPIS_TEST_LIBS})

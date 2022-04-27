project(common_cxx_test)
cmake_minimum_required(VERSION 2.8)
include(cmake/common.cmake)


include_directories(..)

add_library(common_cxx ../string.cxx)

find_package(Threads REQUIRED)

add_definitions(-DVTSS_TEST)

include(CTest)
FIND_PATH(GTEST_INCLUDE_DIR gtest/gtest.h)
FIND_LIBRARY(GTEST_LIBRARY NAMES gtest)
FIND_LIBRARY(GTEST_MAIN_LIBRARY NAMES gtest_main)
set(GTEST_LIBRARIES ${GTEST_LIBRARY} ${GTEST_MAIN_LIBRARY})
if (GTEST_INCLUDE_DIR AND GTEST_LIBRARY AND GTEST_MAIN_LIBRARY)
    message(STATUS "GTEST: ${GTEST_LIBRARIES}, ${GTEST_INCLUDE_DIR}")
else()
    message(FATAL_ERROR "Could not find GTEST")
endif()

MACRO(TEST n)
add_executable(${n} ${n}.cxx)
target_link_libraries(${n}
                      ${GTEST_LIBRARIES}
                      ${CMAKE_THREAD_LIBS_INIT}
                      common_cxx)

add_test(NAME    ${n}
         COMMAND ${n})
ENDMACRO()

TEST(string_test)


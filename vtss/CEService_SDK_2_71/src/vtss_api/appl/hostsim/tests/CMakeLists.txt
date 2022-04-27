include_directories(../src)

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
include_directories(${vtss_api_SOURCE_DIR})

FILE(GLOB TEST_LIB_SRC RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "./base_*.c")
add_library(test_lib ${TEST_LIB_SRC})
target_link_libraries(test_lib ${OPENSSL_LIBRARIES})

function(VTSS_SIMPLE_TEST ARG)
    GET_FILENAME_COMPONENT(BASE ${ARG} NAME_WE)
    add_executable(${BASE} ${ARG})

    target_link_libraries(${BASE} vtss_api venice test_lib)
    ADD_TEST(NAME ${BASE} COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${BASE})
    MESSAGE(STATUS "   * Adding test: ${BASE}")
endfunction()

FILE(GLOB TESTS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "./test_*.c")

FOREACH(TEST ${TESTS})
    VTSS_SIMPLE_TEST(${TEST})
ENDFOREACH(TEST)


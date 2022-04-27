IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

string (REPLACE " -" ";-" CXX_FLAGS        "${CMAKE_CXX_FLAGS}")
string (REPLACE " -" ";-" C_FLAGS          "${CMAKE_C_FLAGS}")
string (REPLACE " -" ";-" EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    LIST(APPEND CXX_FLAGS "-stdlib=libc++")
endif()

LIST(APPEND C_FLAGS   "-Wall")

list(REMOVE_DUPLICATES CXX_FLAGS)
list(REMOVE_DUPLICATES C_FLAGS)
list(REMOVE_DUPLICATES EXE_LINKER_FLAGS)

string (REPLACE ";-" " -" CXX_FLAGS        "${CXX_FLAGS}")
string (REPLACE ";-" " -" C_FLAGS          "${C_FLAGS}")
string (REPLACE ";-" " -" EXE_LINKER_FLAGS "${EXE_LINKER_FLAGS}")

SET(CMAKE_CXX_FLAGS        "${CXX_FLAGS}")
SET(CMAKE_C_FLAGS          "${C_FLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS "${EXE_LINKER_FLAGS}")

message(STATUS "Project name = ${PROJECT_NAME}")
message(STATUS "  Type       = ${CMAKE_BUILD_TYPE}")
message(STATUS "  cxx_flags  = ${CMAKE_CXX_FLAGS}")
message(STATUS "  c_flags    = ${CMAKE_C_FLAGS}")

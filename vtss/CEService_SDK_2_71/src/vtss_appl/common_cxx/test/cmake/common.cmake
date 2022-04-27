enable_testing()

option(ENABLE_COVERAGE "Enable coverage" off)

IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Debug CACHE STRING
        "Choose the type of build, options are: None Debug
        Release RelWithDebInfo MinSizeRel."
        FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

string (REPLACE " " ";" CXX_FLAGS        "${CMAKE_CXX_FLAGS}")
string (REPLACE " " ";" C_FLAGS          "${CMAKE_C_FLAGS}")
string (REPLACE " " ";" EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")

IF(${CMAKE_BUILD_TYPE} MATCHES "Debug")
    LIST(APPEND CXX_FLAGS "-Wall")
    LIST(APPEND C_FLAGS "-Wall")
else()
    LIST(APPEND CXX_FLAGS "-Wall")
    LIST(APPEND C_FLAGS   "-Wall")
endif()

if (${ENABLE_COVERAGE})
    LIST(APPEND CXX_FLAGS        "--coverage")
    LIST(APPEND C_FLAGS          "--coverage")
    LIST(APPEND EXE_LINKER_FLAGS "--coverage")
endif()

list(REMOVE_DUPLICATES CXX_FLAGS)
list(REMOVE_DUPLICATES C_FLAGS)
list(REMOVE_DUPLICATES EXE_LINKER_FLAGS)

string (REPLACE ";" " " CXX_FLAGS "${CXX_FLAGS}")
string (REPLACE ";" " " C_FLAGS "${C_FLAGS}")
string (REPLACE ";" " " EXE_LINKER_FLAGS "${EXE_LINKER_FLAGS}")

SET(CMAKE_CXX_FLAGS        "${CXX_FLAGS}" CACHE STRING "Flags used by the compiler during all build types." FORCE)
SET(CMAKE_C_FLAGS          "${C_FLAGS}" CACHE STRING "Flags used by the compiler during all build types." FORCE)
set(CMAKE_EXE_LINKER_FLAGS "${EXE_LINKER_FLAGS}" CACHE STRING "Flags used by the linker." FORCE)

message(STATUS "Project name =   ${PROJECT_NAME}")
message(STATUS "  Version =      ${${PROJECT_NAME}_VERSION}")
message(STATUS "  Hash =         ${${PROJECT_NAME}_HASH}")
message(STATUS "  Type =         ${CMAKE_BUILD_TYPE}")
message(STATUS "  cxx_flags =    ${CMAKE_CXX_FLAGS}")
message(STATUS "  coverage =     ${ENABLE_COVERAGE}")


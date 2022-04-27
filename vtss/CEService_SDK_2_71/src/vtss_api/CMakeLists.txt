project(vtss_api)

cmake_minimum_required(VERSION 2.8)

include(.cmake/common.cmake)

set(version 4.34)

file(GLOB_RECURSE API_AIL_SRC     RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "base/ail/*.c")
file(GLOB_RECURSE API_B2_SRC      RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "base/b2/*.c")
file(GLOB_RECURSE API_DAYTONA_SRC RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "base/daytona/*.c")
file(GLOB_RECURSE API_JAGUAR1_SRC RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "base/jaguar1/*.c")
file(GLOB_RECURSE API_JAGUAR2_SRC RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "base/jaguar2/*.c")
file(GLOB_RECURSE API_LUTON26_SRC RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "base/luton26/*.c")
file(GLOB_RECURSE API_LUTON28_SRC RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "base/luton28/*.c")
file(GLOB_RECURSE API_PHY_SRC     RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "base/phy/*.c")
file(GLOB_RECURSE API_SERVAL_SRC  RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "base/serval/*.c")
file(GLOB_RECURSE API_INCLUDE     RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "include/*.h")
# NB - boards/ is strictly speaking *not* part of the API! Need to address later...
file(GLOB_RECURSE API_BOARD       RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "boards/*.c")


# Common components which always needs to be includeed
list(APPEND API_SRC ${API_PHY_SRC})
list(APPEND API_SRC ${API_AIL_SRC})

# For now we simply uses jaguar2 to detect if switches are included #
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/base/jaguar2)
option(VTSS_PHY_API_ONLY "Only a PHY API" off)
else()
option(VTSS_PHY_API_ONLY "Only a PHY API" on)
endif()

#cmake is only used on linux targets!
set(VTSS_OPSYS_LINUX 1)
set(VTSS_USE_STDINT_H 1)

# Expanded to defines in the vtss_api_config.h.in file
add_definitions(-DVTSS_HAS_VTSS_API_CONFIG_H)

if (NOT ${VTSS_PHY_API_ONLY})
    set(VTSS_PRODUCT_CHIP "SERVAL"           CACHE STRING "Chip name")
    set(VTSS_PRODUCT_HW   "BOARD_SERVAL_REF" CACHE STRING "Board name")
    option(VTSS_OPT_VCORE_III "Use the internal CPU" on)
#    list(APPEND API_SRC ${API_B2_SRC})
#    list(APPEND API_SRC ${API_DAYTONA_SRC})
    list(APPEND API_SRC ${API_JAGUAR1_SRC})
    list(APPEND API_SRC ${API_JAGUAR2_SRC})
    list(APPEND API_SRC ${API_LUTON26_SRC})
#    list(APPEND API_SRC ${API_LUTON28_SRC})
    list(APPEND API_SRC ${API_SERVAL_SRC})
    list(APPEND API_SRC ${API_BOARD})
    message(STATUS "Building the API for target: ${VTSS_PRODUCT_CHIP}")
else()
    set(VTSS_OPT_VCORE_III 0)
    set(VTSS_PRODUCT_CHIP "PHY"             CACHE STRING "Chip name")
    message(STATUS "Building the API for target: PHY-ONLY")
endif()

# PHY API SETTINGS #############################################################
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/base/phy/phy_10g)
#For now 10g is always off
option(VTSS_CHIP_10G_PHY   "Include 10G PHY support" off)
else ()
option(VTSS_CHIP_10G_PHY   "Include 10G PHY support" off)
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/base/phy/phy_1g)
option(VTSS_CHIP_CU_PHY    "Include CU PHY support" on)
else ()
option(VTSS_CHIP_CU_PHY    "Include CU PHY support" off)
endif ()

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/base/phy/macsec)
option(VTSS_FEATURE_MACSEC "Include MACsec support" on)
else ()
option(VTSS_FEATURE_MACSEC "Include MACsec support" off)
endif()

set(VTSS_OPT_PORT_COUNT    12       CACHE STRING "Port count")

# Install paths
set(BIN_INSTALL_DIR      "bin"     CACHE PATH "Binary install dir")
set(LIB_INSTALL_DIR      "lib"     CACHE PATH "Library install dir")
set(DOC_INSTALL_DIR      "doc"     CACHE PATH "Documentation install dir")
set(INCLUDE_INSTALL_DIR  "include" CACHE PATH "Include install dir")
set(DATA_INSTALL_DIR     "share/${PROJECT_NAME}")
set(CMAKE_INSTALL_DIR    "${DATA_INSTALL_DIR}/cmake")

message(STATUS "BIN_INSTALL_DIR     ${BIN_INSTALL_DIR}")
message(STATUS "LIB_INSTALL_DIR     ${LIB_INSTALL_DIR}")
message(STATUS "DOC_INSTALL_DIR     ${DOC_INSTALL_DIR}")
message(STATUS "INCLUDE_INSTALL_DIR ${INCLUDE_INSTALL_DIR}")
message(STATUS "DATA_INSTALL_DIR    ${DATA_INSTALL_DIR}")
message(STATUS "CMAKE_INSTALL_DIR   ${CMAKE_INSTALL_DIR}")

include_directories(include)

configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/include/vtss_api_config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/vtss_api_config.h"
    ESCAPE_QUOTES)

include_directories("${CMAKE_CURRENT_BINARY_DIR}/include")
list(APPEND API_INCLUDE ${CMAKE_CURRENT_BINARY_DIR}/include/vtss_api_config.h)
add_library(vtss_api SHARED ${API_SRC})

set_target_properties(vtss_api PROPERTIES VERSION ${version} SOVERSION 0)
set_target_properties(vtss_api PROPERTIES PUBLIC_HEADER "${API_INCLUDE}")
set_target_properties(vtss_api PROPERTIES FRAMEWORK TRUE)

install(TARGETS       vtss_api
        EXPORT        ${PROJECT_NAME}Depends
        LIBRARY       DESTINATION "${LIB_INSTALL_DIR}"
        PUBLIC_HEADER DESTINATION "${INCLUDE_INSTALL_DIR}/${PROJECT_NAME}")

# TODO - THIS SHOULD BE DELETED!!!!!!
install(FILES boards/port_custom_api.h
        DESTINATION "${INCLUDE_INSTALL_DIR}/${PROJECT_NAME}/boards")


install(EXPORT        ${PROJECT_NAME}Depends
    DESTINATION   ${CMAKE_INSTALL_DIR} COMPONENT dev)

set(PROJECT_INCLUDE_INSTALL "${CMAKE_INSTALL_PREFIX}/${INCLUDE_INSTALL_DIR}/${PROJECT_NAME}")
configure_file(".cmake/${PROJECT_NAME}-config.cmake.in"
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}-config.cmake" @ONLY)
configure_file(".cmake/${PROJECT_NAME}-config-version.cmake.in"
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}-config-version.cmake" @ONLY)
install(FILES
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}-config.cmake"
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}-config-version.cmake"
  DESTINATION ${CMAKE_INSTALL_DIR} COMPONENT dev)

option(VTSS_APPL_HOSTSIM "Include hostsimulator in build" off)
if (${VTSS_APPL_HOSTSIM})
    add_subdirectory(appl/hostsim)
endif()

option(VTSS_APPL_MINI "Include mini application example in build" off)
if (${VTSS_APPL_MINI})
    add_subdirectory(appl)
endif()

option(GENERATE_DOC "Use Doxygen to create the HTML based API documentation" OFF)
if(GENERATE_DOC)
  FIND_PACKAGE(Doxygen)
  if (NOT DOXYGEN_FOUND)
    message(FATAL_ERROR 
      "Doxygen is needed to build the documentation. Please install it correctly - or don't use GENERATE_DOC")
  endif()
  configure_file(doc/Doxyfile.in ${PROJECT_BINARY_DIR}/Doxyfile)
  add_custom_target (doxygen_${PROJECT_NAME} ALL
    DEPENDS vtss_api
    COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/Doxyfile
    COMMAND ${CMAKE_MAKE_PROGRAM} -C ${PROJECT_BINARY_DIR}/doxygen/latex
    COMMENT "Using doxygen to generate documentation...")
  install(DIRECTORY "${PROJECT_BINARY_DIR}/doxygen/html" DESTINATION "${DOC_INSTALL_DIR}")
  install(FILES "${PROJECT_BINARY_DIR}/doxygen/latex/refman.pdf" DESTINATION "${DOC_INSTALL_DIR}/pdf")
  if(NOT TARGET doxygen)
    add_custom_target(doxygen)
  endif()
  add_dependencies(doxygen doxygen_${PROJECT_NAME})
endif()



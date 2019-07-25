#==============================================================================
# Generic
#==============================================================================

cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project("gabac")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
    message(STATUS "Using fall-back build type '${CMAKE_BUILD_TYPE}'")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)


#==============================================================================
# Compiler
#==============================================================================

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wextra")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")


#==============================================================================
# Executable/library paths
#==============================================================================

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")


#==============================================================================
# Include paths
#==============================================================================

include_directories("${CMAKE_SOURCE_DIR}/source/")
include_directories("${CMAKE_SOURCE_DIR}/source/third-party")


#==============================================================================
# Subdirectories
#==============================================================================

add_subdirectory(source)

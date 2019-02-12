#==============================================================================
# Gabac Lib
#==============================================================================

#==============================================================================
# Project-wide Settings
#==============================================================================

cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

project("gabacify")

# C and C++ code shall be compliant to C(++)11
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

#==============================================================================
# Compiler
#==============================================================================

message(STATUS "CMAKE_CXX_COMPILER_ID=${CMAKE_CXX_COMPILER_ID}")
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wshadow")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wstrict-aliasing")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Intel")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
else()
    message(FATAL_ERROR "C++ compiler currently not supported by CMake build")
endif()

#==============================================================================
# Build type
#==============================================================================

# Set default build type
if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "Build type not specified")
    set(CMAKE_BUILD_TYPE Debug)
    message(STATUS "Using fall-back build type '${CMAKE_BUILD_TYPE}'")
endif()

# Log the compiler flags for each build type
message(STATUS "C++ compiler flags: ${CMAKE_CXX_FLAGS}")
message(STATUS "Additional C++ compiler flags for build type 'Debug': ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "Additional C++ compiler flags for build type 'Release': ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "Additional C++ compiler flags for build type 'RelWithDebInfo': ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
message(STATUS "Additional C++ compiler flags for build type 'MinSizeRel': ${CMAKE_CXX_FLAGS_MINSIZEREL}")
message(STATUS "Selected build type: ${CMAKE_BUILD_TYPE}")


#==============================================================================
# Boost
#==============================================================================

find_package(Boost 1.54.0 REQUIRED COMPONENTS regex program_options)

#==============================================================================
# gabacify
#==============================================================================

# Set the paths
set(gabacify "gabacify")
set(gabacify_source_dir ${PROJECT_SOURCE_DIR})
set(gabacify_header_dir ${PROJECT_SOURCE_DIR})

get_filename_component(PARENT_DIR ${PROJECT_SOURCE_DIR} DIRECTORY)
set(gabacify_include_dir ${PARENT_DIR})

# List all source files (alphabetically)
set(gabacify_source_files ${gabacify_source_files} ${gabacify_source_dir}/analyze.cpp)
set(gabacify_source_files ${gabacify_source_files} ${gabacify_source_dir}/decode.cpp)
set(gabacify_source_files ${gabacify_source_files} ${gabacify_source_dir}/encode.cpp)
set(gabacify_source_files ${gabacify_source_files} ${gabacify_source_dir}/file.cpp)
set(gabacify_source_files ${gabacify_source_files} ${gabacify_source_dir}/input_file.cpp)
set(gabacify_source_files ${gabacify_source_files} ${gabacify_source_dir}/main.cpp)
set(gabacify_source_files ${gabacify_source_files} ${gabacify_source_dir}/output_file.cpp)
set(gabacify_source_files ${gabacify_source_files} ${gabacify_source_dir}/program_options.cpp)
set(gabacify_header_files ${gabacify_header_files} ${gabacify_header_dir}/tmp_file.cpp)

# List all header files (alphabetically)
set(gabacify_source_files ${gabacify_source_files} ${gabacify_source_dir}/analyze.h)
set(gabacify_header_files ${gabacify_header_files} ${gabacify_header_dir}/decode.h)
set(gabacify_header_files ${gabacify_header_files} ${gabacify_header_dir}/encode.h)
set(gabacify_header_files ${gabacify_header_files} ${gabacify_header_dir}/file.h)
set(gabacify_header_files ${gabacify_header_files} ${gabacify_header_dir}/input_file.h)
set(gabacify_header_files ${gabacify_header_files} ${gabacify_header_dir}/output_file.h)
set(gabacify_header_files ${gabacify_header_files} ${gabacify_header_dir}/program_options.h)
set(gabacify_header_files ${gabacify_header_files} ${gabacify_header_dir}/tmp_file.h)

# Group the source and header files
source_group("gabacify Source Files" FILES ${gabacify_source_files})
source_group("gabacify Header Files" FILES ${gabacify_header_files})

# Set up the target
add_executable(${gabacify} EXCLUDE_FROM_ALL ${gabacify_source_files} ${gabacify_header_files})
target_include_directories(${gabacify} PRIVATE ${gabacify_include_dir})
target_include_directories(${gabacify} PRIVATE ${PARENT_DIR}/gabac})
target_link_libraries(${gabacify} gabac)
target_link_libraries(${gabacify} Boost::program_options)


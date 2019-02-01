#==============================================================================
# Generic
#==============================================================================

# We require at least CMake 3.1
cmake_minimum_required(VERSION 3.1)
message(STATUS "CMake generator: ${CMAKE_GENERATOR}")

# C and C++ code shall be compliant to C(++)11
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

# This is gabac
project("gabac")

# Custom CMake cache variables
set(gabac OFF CACHE BOOL "Build shared GABAC library.")
message(STATUS "GABAC_BUILD_SHARED_LIB=${GABAC_BUILD_SHARED_LIB}")

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

find_package(Boost 1.54 COMPONENTS regex log program_options REQUIRED)


#==============================================================================
# Google Test
#==============================================================================

# Download and unpack googletest at configure time
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(
    COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download
)
if(result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(
    COMMAND ${CMAKE_COMMAND} --build .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download
)
if(result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines the gtest and gtest_main
# targets.
add_subdirectory(
    ${CMAKE_CURRENT_BINARY_DIR}/googletest-src
    ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
    EXCLUDE_FROM_ALL
)

# The gtest/gtest_main targets carry header search path dependencies
# automatically when using CMake 2.8.11 or later. Otherwise we have to add
# them here ourselves.
if(CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${gtest_SOURCE_DIR}/include")
endif()


#==============================================================================
# gabac
#==============================================================================

# Set the paths
set(gabac "gabac")
set(gabac_source_dir ${CMAKE_SOURCE_DIR}/source/gabac)
set(gabac_header_dir ${CMAKE_SOURCE_DIR}/source/gabac)
set(gabac_include_dir ${CMAKE_SOURCE_DIR}/source)

# List all source files (alphabetically)
set(gabac_source_files ${gabac_source_files} ${gabac_source_dir}/binary_arithmetic_encoder.cpp)
set(gabac_source_files ${gabac_source_files} ${gabac_source_dir}/binary_arithmetic_decoder.cpp)
set(gabac_source_files ${gabac_source_files} ${gabac_source_dir}/bit_input_stream.cpp)
set(gabac_source_files ${gabac_source_files} ${gabac_source_dir}/bit_output_stream.cpp)
set(gabac_source_files ${gabac_source_files} ${gabac_source_dir}/constants.cpp)
set(gabac_source_files ${gabac_source_files} ${gabac_source_dir}/context_model.cpp)
set(gabac_source_files ${gabac_source_files} ${gabac_source_dir}/context_selector.cpp)
set(gabac_source_files ${gabac_source_files} ${gabac_source_dir}/context_tables.cpp)
set(gabac_source_files ${gabac_source_files} ${gabac_source_dir}/decoding.cpp)
set(gabac_source_files ${gabac_source_files} ${gabac_source_dir}/diff_coding.cpp)
set(gabac_source_files ${gabac_source_files} ${gabac_source_dir}/encoding.cpp)
set(gabac_source_files ${gabac_source_files} ${gabac_source_dir}/equality_coding.cpp)
set(gabac_source_files ${gabac_source_files} ${gabac_source_dir}/lut_transform.cpp)
set(gabac_source_files ${gabac_source_files} ${gabac_source_dir}/match_coding.cpp)
set(gabac_source_files ${gabac_source_files} ${gabac_source_dir}/reader.cpp)
set(gabac_source_files ${gabac_source_files} ${gabac_source_dir}/release.cpp)
set(gabac_source_files ${gabac_source_files} ${gabac_source_dir}/rle_coding.cpp)
set(gabac_source_files ${gabac_source_files} ${gabac_source_dir}/writer.cpp)

# List all header files (alphabetically)
set(gabac_header_files ${gabac_header_files} ${gabac_header_dir}/binary_arithmetic_encoder.h)
set(gabac_header_files ${gabac_header_files} ${gabac_header_dir}/binary_arithmetic_decoder.h)
set(gabac_header_files ${gabac_header_files} ${gabac_header_dir}/bit_input_stream.h)
set(gabac_header_files ${gabac_header_files} ${gabac_header_dir}/bit_output_stream.h)
set(gabac_header_files ${gabac_header_files} ${gabac_header_dir}/cabac_tables.h)
set(gabac_header_files ${gabac_header_files} ${gabac_header_dir}/constants.h)
set(gabac_header_files ${gabac_header_files} ${gabac_header_dir}/context_model.h)
set(gabac_header_files ${gabac_header_files} ${gabac_header_dir}/context_selector.h)
set(gabac_header_files ${gabac_header_files} ${gabac_header_dir}/context_tables.h)
set(gabac_header_files ${gabac_header_files} ${gabac_header_dir}/decoding.h)
set(gabac_header_files ${gabac_header_files} ${gabac_header_dir}/diff_coding.h)
set(gabac_header_files ${gabac_header_files} ${gabac_header_dir}/encoding.h)
set(gabac_header_files ${gabac_header_files} ${gabac_header_dir}/equality_coding.h)
set(gabac_header_files ${gabac_header_files} ${gabac_header_dir}/gabac.h)
set(gabac_header_files ${gabac_header_files} ${gabac_header_dir}/lut_transform.h)
set(gabac_header_files ${gabac_header_files} ${gabac_header_dir}/match_coding.h)
set(gabac_header_files ${gabac_header_files} ${gabac_header_dir}/reader.h)
set(gabac_header_files ${gabac_header_files} ${gabac_header_dir}/release.h)
set(gabac_header_files ${gabac_header_files} ${gabac_header_dir}/return_codes.h)
set(gabac_header_files ${gabac_header_files} ${gabac_header_dir}/rle_coding.h)
set(gabac_header_files ${gabac_header_files} ${gabac_header_dir}/writer.h source/gabac/constants.cpp source/gabac/data_stream.cpp source/gabac/data_stream.h)

# Group the source and header files
source_group("GABAC Source Files" FILES ${gabac_source_files})
source_group("GABAC Header Files" FILES ${gabac_header_files})

# Set up the target
if(${GABAC_BUILD_SHARED_LIB})
    message(STATUS "Configured build of GABAC as *shared* library")
    add_library(${gabac} SHARED ${gabac_source_files} ${gabac_header_files})
    set_target_properties(${gabac} PROPERTIES POSITION_INDEPENDENT_CODE 1)
else()
    message(STATUS "Configured build of GABAC as *static* library")
    add_library(${gabac} STATIC ${gabac_source_files} ${gabac_header_files})
endif()
target_include_directories(${gabac} PRIVATE ${gabac_include_dir})


#==============================================================================
# gabacify
#==============================================================================

# Set the paths
set(gabacify "gabacify")
set(gabacify_source_dir ${CMAKE_SOURCE_DIR}/source/gabacify)
set(gabacify_header_dir ${CMAKE_SOURCE_DIR}/source/gabacify)
set(gabacify_include_dir ${CMAKE_SOURCE_DIR}/source)

# List all source files (alphabetically)
set(gabacify_source_files ${gabacify_source_files} ${gabacify_source_dir}/analysis.cpp)
set(gabacify_source_files ${gabacify_source_files} ${gabacify_source_dir}/configuration.cpp)
set(gabacify_source_files ${gabacify_source_files} ${gabacify_source_dir}/decode.cpp)
set(gabacify_source_files ${gabacify_source_files} ${gabacify_source_dir}/encode.cpp)
set(gabacify_source_files ${gabacify_source_files} ${gabacify_source_dir}/exceptions.cpp)
set(gabacify_source_files ${gabacify_source_files} ${gabacify_source_dir}/file.cpp)
set(gabacify_source_files ${gabacify_source_files} ${gabacify_source_dir}/helpers.cpp)
set(gabacify_source_files ${gabacify_source_files} ${gabacify_source_dir}/input_file.cpp)
set(gabacify_source_files ${gabacify_source_files} ${gabacify_source_dir}/log.cpp)
set(gabacify_source_files ${gabacify_source_files} ${gabacify_source_dir}/main.cpp)
set(gabacify_source_files ${gabacify_source_files} ${gabacify_source_dir}/output_file.cpp)
set(gabacify_source_files ${gabacify_source_files} ${gabacify_source_dir}/program_options.cpp)
set(gabacify_header_files ${gabacify_header_files} ${gabacify_header_dir}/tmp_file.cpp)

# List all header files (alphabetically)
set(gabacify_header_files ${gabacify_header_files} ${gabacify_header_dir}/analysis.h)
set(gabacify_header_files ${gabacify_header_files} ${gabacify_header_dir}/configuration.h)
set(gabacify_header_files ${gabacify_header_files} ${gabacify_header_dir}/decode.h)
set(gabacify_header_files ${gabacify_header_files} ${gabacify_header_dir}/encode.h)
set(gabacify_header_files ${gabacify_header_files} ${gabacify_header_dir}/exceptions.h)
set(gabacify_header_files ${gabacify_header_files} ${gabacify_header_dir}/file.h)
set(gabacify_header_files ${gabacify_header_files} ${gabacify_header_dir}/helpers.h)
set(gabacify_header_files ${gabacify_header_files} ${gabacify_header_dir}/input_file.h)
set(gabacify_header_files ${gabacify_header_files} ${gabacify_header_dir}/log.h)
set(gabacify_header_files ${gabacify_header_files} ${gabacify_header_dir}/output_file.h)
set(gabacify_header_files ${gabacify_header_files} ${gabacify_header_dir}/program_options.h)
set(gabacify_header_files ${gabacify_header_files} ${gabacify_header_dir}/tmp_file.h)

# Group the source and header files
source_group("gabacify Source Files" FILES ${gabacify_source_files})
source_group("gabacify Header Files" FILES ${gabacify_header_files})

# Set up the target
add_executable(${gabacify} EXCLUDE_FROM_ALL ${gabacify_source_files} ${gabacify_header_files})
target_compile_definitions(${gabacify} PRIVATE BOOST_LOG_DYN_LINK)
target_include_directories(${gabacify} PRIVATE ${gabacify_include_dir})
target_include_directories(${gabacify} PRIVATE ${gabac_include_dir})
target_link_libraries(${gabacify} ${gabac})
target_link_libraries(${gabacify} Boost::log)
target_link_libraries(${gabacify} Boost::program_options)


#==============================================================================
# tests
#==============================================================================

# Set the paths
set(tests "tests")
set(tests_source_dir ${CMAKE_SOURCE_DIR}/tests)
set(tests_header_dir ${CMAKE_SOURCE_DIR}/tests)
set(tests_include_dir ${CMAKE_SOURCE_DIR})

# List all source files (alphabetically)
set(tests_source_files ${tests_source_files} ${tests_source_dir}/example_test.cpp)
set(tests_source_files ${tests_source_files} ${tests_source_dir}/main.cpp)
#
set(tests_source_files ${tests_source_files} ${tests_source_dir}/gabac/binary_arithmetic_decoder_test.cpp)
set(tests_source_files ${tests_source_files} ${tests_source_dir}/gabac/bit_input_stream_test.cpp)
set(tests_source_files ${tests_source_files} ${tests_source_dir}/gabac/bit_output_stream_test.cpp)
set(tests_source_files ${tests_source_files} ${tests_source_dir}/gabac/core_test.cpp)
set(tests_source_files ${tests_source_files} ${tests_source_dir}/gabac/diff_coding_test.cpp)
set(tests_source_files ${tests_source_files} ${tests_source_dir}/gabac/equality_coding_test.cpp)
set(tests_source_files ${tests_source_files} ${tests_source_dir}/gabac/lut_transform_test.cpp)
set(tests_source_files ${tests_source_files} ${tests_source_dir}/gabac/match_coding_test.cpp)
set(tests_source_files ${tests_source_files} ${tests_source_dir}/gabac/rle_coding_test.cpp)

# List all header files (alphabetically)
set(tests_header_files ${tests_header_files} ${tests_header_dir}/gabac/test_common.h)

# Group the source and header files
source_group("Tests Source Files" FILES ${tests_source_files})
source_group("Tests Header Files" FILES ${tests_header_files})

# Set up the target
add_executable(${tests} EXCLUDE_FROM_ALL ${tests_source_files})
target_include_directories(${tests} PRIVATE ${gabac_include_dir})
target_link_libraries(${tests} gtest_main)
target_link_libraries(${tests} ${gabac})

cmake_minimum_required(VERSION 3.10.0)
project (StunServerMiniApp)

# Default cmake modules may not be found from under docker container, so keep them in this folder
#include(CMakePrintHelpers)
include(./CMakePrintHelpers.cmake)

message("========== ${PROJECT_NAME} [start] ==========")
cmake_print_variables(PROJECT_SOURCE_DIR)
cmake_print_variables(CMAKE_SYSTEM_NAME)
message("")

set(CMAKE_BUILD_TYPE Release)
set(ROOT_PROJECT_DIR ${PROJECT_SOURCE_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Predefined macros
add_compile_definitions(__CMAKE_BUILD__)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_compile_definitions(__WINDOWS_OS__)
endif()

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    add_compile_definitions(__UNIX_OS__)
endif()

# Build executables
add_subdirectory(StunServerMini)

message("========== ${PROJECT_NAME} [end] ==========")
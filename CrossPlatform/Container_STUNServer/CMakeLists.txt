cmake_minimum_required(VERSION 3.10.0)
project(STUNServerApp)

include(./CMakePrintHelpers.cmake)

message("========== ${PROJECT_NAME} [start] ==========")
cmake_print_variables(PROJECT_SOURCE_DIR)
message("")

set(CMAKE_BUILD_TYPE Release)
set(ROOT_PROJECT_DIR ${PROJECT_SOURCE_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Build all libraries (set global names to have access in child scripts)
# ...

# Build executables
add_subdirectory(STUNServer)

message("========== ${PROJECT_NAME} [end] ==========")
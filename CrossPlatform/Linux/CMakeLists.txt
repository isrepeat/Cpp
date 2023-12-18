cmake_minimum_required(VERSION 3.10.0)
include(CMakePrintHelpers)

project (Linux_App)
message("========== ${PROJECT_NAME} [start] ==========")
cmake_print_variables(PROJECT_SOURCE_DIR)
message("")

set(ROOT_PROJECT_DIR ${PROJECT_SOURCE_DIR})

# Build all libraries (set global names to have access in child scripts)
set(TestStaticLib "Linux_Test.Static")
set(TestDynamicLib "Linux_Test.Dynamic")
#add_subdirectory(${TestStaticLib})
add_subdirectory(${TestDynamicLib})

# Build executables
add_subdirectory(Linux_Test.App)

message("========== ${PROJECT_NAME} [end] ==========")
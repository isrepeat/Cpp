cmake_minimum_required(VERSION 3.10.0)

project(Linux_Test.Static)
message("========== ${PROJECT_NAME} [start] ==========")
cmake_print_variables(PROJECT_NAME)
cmake_print_variables(PROJECT_SOURCE_DIR)
message("")

# find packages ...

file(GLOB PROJECT_SOURCES "*.h" "*.cpp")
add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

# share project dirs for those who link library
target_include_directories(${PROJECT_NAME} PUBLIC
	"${PROJECT_SOURCE_DIR}"
)
message("========== ${PROJECT_NAME} [end] ==========")
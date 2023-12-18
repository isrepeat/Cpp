cmake_minimum_required(VERSION 3.10.0)
# ${TestStaticLib} must already be added as library
# ${TestDynamicLib} must already be added as library

project(Linux_Test.App)
message("========== ${PROJECT_NAME} [start] ==========")
cmake_print_variables(PROJECT_SOURCE_DIR)
message("")

# find packages ...

file(GLOB PROJECT_SOURCES "*.h" "*.cpp")
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_link_libraries(${PROJECT_NAME} ${TestDynamicLib})
message("========== ${PROJECT_NAME} [end] ==========")
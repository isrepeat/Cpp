cmake_minimum_required(VERSION 3.10.0)
# Ensure you have:
#   - 3rdParty/asio
#   - 3rdParty/MagicEnum
#   - packages/Logger.1.2.9

project(StunServerMini)
message("========== ${PROJECT_NAME} [start] ==========")
cmake_print_variables(PROJECT_SOURCE_DIR)
cmake_print_variables(PROJECT_BINARY_DIR)
cmake_print_variables(CMAKE_RUNTIME_OUTPUT_DIRECTORY)
message("")

file(GLOB STUN_SERVER_SRCS "*.h" "*.cpp")

#find_package(OpenSSL REQUIRED)

include_directories(
    ${PROJECT_BINARY_DIR} 
    ${PROTOBUF_INCLUDE_DIRS}
    ${ROOT_PROJECT_DIR}/3rdParty
    ${ROOT_PROJECT_DIR}/3rdParty/MagicEnum
    ${ROOT_PROJECT_DIR}/3rdParty/asio/include
    ${ROOT_PROJECT_DIR}/packages/Logger.1.2.9/build/native/include
    ${ROOT_PROJECT_DIR}/packages/Logger.1.2.9/build/native/include/Logger
    ${ROOT_PROJECT_DIR}/packages/Logger.1.2.9/build/native/include/Logger/spdlog
)

add_executable(${PROJECT_NAME} ${STUN_SERVER_SRCS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fpermissive")
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)


add_library(LoggerDesktop SHARED IMPORTED GLOBAL)
set_property(TARGET LoggerDesktop PROPERTY IMPORTED_IMPLIB_RELEASE "${ROOT_PROJECT_DIR}/packages/Logger.1.2.9/build/x64/Release/LoggerDesktop.lib") # Ensure the path is correct
set_property(TARGET LoggerDesktop PROPERTY IMPORTED_LOCATION_RELEASE "${ROOT_PROJECT_DIR}/packages/Logger.1.2.9/build/x64/Release/LoggerDesktop.dll") # Ensure the path is correct

target_link_libraries(${PROJECT_NAME} -pthread -lrt -ldl LoggerDesktop)
#target_link_libraries(${PROJECT_NAME} -pthread -lrt -ldl LoggerDesktop OpenSSL::Crypto)

# Copy LoggerDesktop.dll to bin dir
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:LoggerDesktop> $<TARGET_FILE_DIR:${PROJECT_NAME}>
)
message("========== ${PROJECT_NAME} [end] ==========")
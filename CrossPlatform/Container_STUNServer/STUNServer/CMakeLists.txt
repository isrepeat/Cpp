cmake_minimum_required(VERSION 3.10.0)
# Ensure you have installed/built:
#   - p7
#   - msquic (v1.2.0)
#   - protobuf package

project(STUNServer)
message("========== ${PROJECT_NAME} [start] ==========")
cmake_print_variables(PROJECT_SOURCE_DIR)
cmake_print_variables(PROJECT_BINARY_DIR)
cmake_print_variables(CMAKE_RUNTIME_OUTPUT_DIRECTORY)
message("")

file(GLOB PROTO_FILES "ProtoFiles/*.proto")
file(GLOB STUN_SERVER_SRCS "*.h" "*.cpp")

find_package(Protobuf REQUIRED)
find_package(OpenSSL REQUIRED)

cmake_print_variables(PROTOBUF_INCLUDE_DIRS)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES}) # generated in PROJECT_BINARY_DIR

include_directories(
    ${ROOT_PROJECT_DIR}/3rdParty/msquic/release_1.2.0/include
    ${ROOT_PROJECT_DIR}/3rdParty/asio/include
    ${ROOT_PROJECT_DIR}/3rdParty/p7/Headers
    ${ROOT_PROJECT_DIR}/3rdParty 
    ${PROTOBUF_INCLUDE_DIRS}
    ${PROJECT_BINARY_DIR} 
)

add_executable(${PROJECT_NAME} ${PROTO_SRCS} ${STUN_SERVER_SRCS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fpermissive")
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

add_library(msquic UNKNOWN IMPORTED)
set_property(TARGET msquic PROPERTY IMPORTED_LOCATION "${ROOT_PROJECT_DIR}/3rdParty/msquic/release_1.2.0/bin/libmsquic.so") # Ensure the path is correct

find_library(p7_LIBRARY NAMES p7 REQUIRED)
cmake_print_variables(p7_LIBRARY)
add_library(p7 UNKNOWN IMPORTED)
set_property(TARGET p7 PROPERTY IMPORTED_LOCATION "${p7_LIBRARY}")

target_link_libraries(${PROJECT_NAME} -pthread -lrt -ldl p7 msquic protobuf::libprotobuf OpenSSL::Crypto)

file(COPY "./host.cert" "./host.key" DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
message("========== ${PROJECT_NAME} [end] ==========")
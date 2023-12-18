cmake_minimum_required(VERSION 3.10.0)

project(STUNServer)
message("========== ${PROJECT_NAME} [start] ==========")
cmake_print_variables(PROJECT_SOURCE_DIR)
message("")

file(GLOB PROTO_FILES "ProtoFiles/*.proto")
file(GLOB STUN_SERVER_SRCS "*.h" "*.cpp")
#set(STUN_SERVER_SRCS "TestHello.cpp")

# find packages ...
find_package(Protobuf REQUIRED)
find_package(OpenSSL REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

include_directories(
    ${ROOT_PROJECT_DIR}/3rdParty/msquic/src/inc
    ${ROOT_PROJECT_DIR}/3rdParty/asio/include
    ${ROOT_PROJECT_DIR}/3rdParty/p7/include
    ${ROOT_PROJECT_DIR}/3rdParty 
    ${PROTOBUF_INCLUDE_DIRS}
    ${PROTO_HDRS} 
)

add_executable(${PROJECT_NAME} ${PROTO_SRCS} ${STUN_SERVER_SRCS})
#add_executable(${PROJECT_NAME} ${STUN_SERVER_SRCS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fpermissive")
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

add_library(msquic UNKNOWN IMPORTED)
set_property(TARGET msquic PROPERTY IMPORTED_LOCATION "${ROOT_PROJECT_DIR}/3rdParty/msquic/bin/linux_x64_Release_openssl/libmsquic.so")


find_library(p7_LIBRARY NAMES p7 REQUIRED)
cmake_print_variables(p7_LIBRARY)

add_library(p7 UNKNOWN IMPORTED)
set_property(TARGET p7 PROPERTY IMPORTED_LOCATION "${p7_LIBRARY}")

target_link_libraries(${PROJECT_NAME} -pthread -lrt -ldl -lbacktrace p7 msquic protobuf::libprotobuf OpenSSL::Crypto)
message("========== ${PROJECT_NAME} [end] ==========")
cmake_minimum_required(VERSION 3.10.0)
# ${TestStaticLib} must already be added as library
# ${TestDynamicLib} must already be added as library

project(STUNServer)
message("========== ${PROJECT_NAME} [start] ==========")
cmake_print_variables(PROJECT_SOURCE_DIR)
message("")

file(GLOB PROTO_FILES "*.proto")
#file(GLOB STUN_SERVER_SRCS "*.h" "*.cpp")
set(STUN_SERVER_SRCS "TestHello.cpp")

# find packages ...
#find_package(Protobuf REQUIRED)
#find_package(OpenSSL REQUIRED)

#protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

#add_executable(${PROJECT_NAME} ${PROTO_SRCS} ${STUN_SERVER_SRCS})
add_executable(${PROJECT_NAME} ${STUN_SERVER_SRCS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fpermissive")
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

#add_library(msquic UNKNOWN IMPORTED)
#set_property(TARGET msquic PROPERTY IMPORTED_LOCATION "/home/npv_nimda_cca/STUNServer/3rdParty/msquic/artifacts/bin/linux/x64_Release_openssl/libmsquic.so")

#find_library(p7_REL NAMES p7)
#add_library(p7 UNKNOWN IMPORTED)
#set_property(TARGET p7 PROPERTY IMPORTED_LOCATION "${p7_REL}")

#target_link_libraries(STUNServer -pthread -lrt -ldl -lbacktrace msquic p7 protobuf::libprotobuf OpenSSL::Crypto)

#target_link_libraries(${PROJECT_NAME} ${TestDynamicLib})
message("========== ${PROJECT_NAME} [end] ==========")
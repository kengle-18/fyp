cmake_minimum_required(VERSION 3.20)
project(HelloGrpc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Use system packages instead of FetchContent
find_package(PkgConfig REQUIRED)
find_package(Protobuf REQUIRED)

# Find system protoc and grpc plugin
find_program(PROTOC protoc REQUIRED)
find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin REQUIRED)

# -------------------------------
# Proto file processing - Use system protobuf
# -------------------------------
get_filename_component(ROOT_DIR "${CMAKE_SOURCE_DIR}" PATH)
set(PROTO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(GENERATED_DIR ${CMAKE_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_DIR})

file(GLOB PROTO_FILES "${PROTO_DIR}/*.proto")

set(PROTO_GEN_SRCS "")

foreach(proto ${PROTO_FILES})
    get_filename_component(proto_name ${proto} NAME_WE)

    set(proto_srcs "${GENERATED_DIR}/${proto_name}.pb.cc")
    set(proto_hdrs "${GENERATED_DIR}/${proto_name}.pb.h")
    set(grpc_srcs "${GENERATED_DIR}/${proto_name}.grpc.pb.cc")
    set(grpc_hdrs "${GENERATED_DIR}/${proto_name}.grpc.pb.h")

    add_custom_command(
        OUTPUT ${proto_srcs} ${proto_hdrs} ${grpc_srcs} ${grpc_hdrs}
        COMMAND ${PROTOC}
            -I ${PROTO_DIR}
            --cpp_out=${GENERATED_DIR}
            --grpc_out=${GENERATED_DIR}
            --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
            ${proto}
        DEPENDS ${proto}
        COMMENT "Generating C++ and gRPC sources from ${proto}"
        VERBATIM
    )

    list(APPEND PROTO_GEN_SRCS ${proto_srcs} ${grpc_srcs})
endforeach()

# -------------------------------
# Main executable
# -------------------------------
add_executable(hello_grpc
    src/main.cpp
    ${PROTO_GEN_SRCS}
)

target_include_directories(hello_grpc 
    PRIVATE 
    ${GENERATED_DIR}
)

# Debug: Print available info
message(STATUS "📌 PROTO_GEN_SRCS: ${PROTO_GEN_SRCS}")
message(STATUS "📌 gRPC version: ${gRPC_VERSION}")

# Link against system gRPC
target_link_libraries(hello_grpc
    PRIVATE
    grpc++
    grpc
    protobuf::libprotobuf
)
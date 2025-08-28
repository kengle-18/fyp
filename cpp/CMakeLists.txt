cmake_minimum_required(VERSION 3.20)
project(HelloGrpc LANGUAGES CXX)

# -------------------------------
# Global settings
# -------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)  # Needed for shared libs on macOS

include(FetchContent)

# -------------------------------
# 1️⃣ Fetch Protobuf
# -------------------------------
message(STATUS "🔄 Fetching Protobuf...")
FetchContent_Declare(
    protobuf
    GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
    GIT_TAG v29.3
    SOURCE_SUBDIR cmake
)
set(protobuf_BUILD_TESTS OFF)
set(protobuf_INSTALL OFF)
FetchContent_MakeAvailable(protobuf)

# -------------------------------
# 2️⃣ Fetch gRPC
# -------------------------------
message(STATUS "🔄 Fetching gRPC...")
FetchContent_Declare(
    grpc
    GIT_REPOSITORY https://github.com/grpc/grpc.git
    GIT_TAG v1.66.1
    SOURCE_SUBDIR cmake
)
set(gRPC_BUILD_TESTS OFF)
set(gRPC_INSTALL OFF)
FetchContent_MakeAvailable(grpc)

# -------------------------------
# 3️⃣ Protoc and gRPC plugin executables
# -------------------------------
if(WIN32)
    set(PROTOC_EXECUTABLE "${protobuf_BINARY_DIR}/protoc.exe")
    set(GRPC_CPP_PLUGIN_EXECUTABLE "${grpc_BINARY_DIR}/grpc_cpp_plugin.exe")
else()
    set(PROTOC_EXECUTABLE "${protobuf_BINARY_DIR}/protoc")
    set(GRPC_CPP_PLUGIN_EXECUTABLE "${grpc_BINARY_DIR}/grpc_cpp_plugin")
endif()

message(STATUS "Using protoc: ${PROTOC_EXECUTABLE}")
message(STATUS "Using grpc_cpp_plugin: ${GRPC_CPP_PLUGIN_EXECUTABLE}")

# -------------------------------
# 4️⃣ Proto files
# -------------------------------
set(PROTO_DIR "${CMAKE_SOURCE_DIR}/../scala/src/main/protobuf")
file(GLOB PROTO_FILES "${PROTO_DIR}/*.proto")

set(GENERATED_DIR "${CMAKE_BINARY_DIR}/generated")
file(MAKE_DIRECTORY ${GENERATED_DIR})

set(PROTO_GEN_SRCS "")

foreach(proto ${PROTO_FILES})
    get_filename_component(proto_name ${proto} NAME_WE)

    # Generated files
    set(proto_src "${GENERATED_DIR}/${proto_name}.pb.cc")
    set(proto_hdr "${GENERATED_DIR}/${proto_name}.pb.h")
    set(grpc_src "${GENERATED_DIR}/${proto_name}.grpc.pb.cc")
    set(grpc_hdr "${GENERATED_DIR}/${proto_name}.grpc.pb.h")

    # Custom command to generate files
    add_custom_command(
        OUTPUT ${proto_src} ${proto_hdr} ${grpc_src} ${grpc_hdr}
        COMMAND ${PROTOC_EXECUTABLE}
            -I ${PROTO_DIR}
            --cpp_out=${GENERATED_DIR}
            --grpc_out=${GENERATED_DIR}
            --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN_EXECUTABLE}
            ${proto}
        DEPENDS ${proto} ${PROTOC_EXECUTABLE} ${GRPC_CPP_PLUGIN_EXECUTABLE}
        COMMENT "Generating C++ and gRPC sources from ${proto}"
        VERBATIM
    )

    list(APPEND PROTO_GEN_SRCS ${proto_src} ${grpc_src})
endforeach()

# -------------------------------
# 5️⃣ Main executable
# -------------------------------
file(GLOB CPP_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(hello_grpc
    ${CPP_FILES}
    ${PROTO_GEN_SRCS}
)

target_include_directories(hello_grpc PRIVATE ${GENERATED_DIR})

# Link libraries built from FetchContent
if(WIN32)
    target_link_libraries(hello_grpc
        PRIVATE
        "${grpc_BINARY_DIR}/Release/grpc++.lib"
        "${grpc_BINARY_DIR}/Release/grpc.lib"
        "${protobuf_BINARY_DIR}/Release/libprotobuf.lib"
    )
else()
    target_link_libraries(hello_grpc
        PRIVATE
        grpc++
        grpc
        protobuf
    )
endif()

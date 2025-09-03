cmake_minimum_required(VERSION 3.20)
project(HelloGrpc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Disable all testing and shared libraries BEFORE FetchContent
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libraries" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "Disable testing" FORCE)

# gRPC settings
set(gRPC_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(gRPC_BUILD_CSHARP_EXT OFF CACHE BOOL "" FORCE)
set(gRPC_INSTALL OFF CACHE BOOL "Disable gRPC install" FORCE)

# Abseil settings (gRPC will fetch this)
set(ABSL_BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(ABSL_BUILD_TEST_HELPERS OFF CACHE BOOL "" FORCE)
set(ABSL_USE_GOOGLETEST_HEAD OFF CACHE BOOL "" FORCE)
set(ABSL_ENABLE_INSTALL OFF CACHE BOOL "Disable Abseil install" FORCE)

# Protobuf settings (gRPC includes this as third_party)
set(protobuf_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(protobuf_BUILD_PROTOC_BINARIES ON CACHE BOOL "" FORCE)
set(protobuf_INSTALL OFF CACHE BOOL "Disable protobuf install" FORCE)
set(utf8_range_ENABLE_INSTALL OFF CACHE BOOL "Disable utf8_range install" FORCE)

include(FetchContent)

# -------------------------------
# Fetch gRPC (this includes protobuf as third_party)
# -------------------------------
FetchContent_Declare(
    grpc
    GIT_REPOSITORY https://github.com/grpc/grpc.git
    GIT_TAG v1.50.0
)
FetchContent_MakeAvailable(grpc)

# -------------------------------
# Proto file processing - Use gRPC's built-in protobuf
# -------------------------------
get_filename_component(ROOT_DIR "${CMAKE_SOURCE_DIR}" PATH)
set(PROTO_DIR "${ROOT_DIR}/scala/src/main/protobuf")
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
        COMMAND $<TARGET_FILE:protoc>
            -I ${PROTO_DIR}
            --cpp_out=${GENERATED_DIR}
            --grpc_out=${GENERATED_DIR}
            --plugin=protoc-gen-grpc=$<TARGET_FILE:grpc_cpp_plugin>
            ${proto}
        DEPENDS ${proto} protoc grpc_cpp_plugin
        COMMENT "Generating C++ and gRPC sources from ${proto}"
        VERBATIM
    )

    list(APPEND PROTO_GEN_SRCS ${proto_srcs} ${grpc_srcs})
endforeach()

# -------------------------------
# Main executable
# -------------------------------
# file(GLOB CPP_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(hello_grpc
    src/main.cpp
    ${PROTO_GEN_SRCS}
)

target_include_directories(hello_grpc 
    PRIVATE 
    ${GENERATED_DIR}
)

# Debug: Print available targets
get_property(allTargets DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)
message(STATUS "📌 Available targets: ${allTargets}")
message(STATUS "📌 PROTO_GEN_SRCS: ${PROTO_GEN_SRCS}")

# Link against the same protobuf that gRPC uses
# Note: gRPC v1.71.0 uses these target names
target_link_libraries(hello_grpc
    PRIVATE
    grpc++
    grpc
    libprotobuf
)
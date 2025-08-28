cmake_minimum_required(VERSION 3.10)
project(hello-cpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Use FetchContent to automatically download and configure gRPC and its dependencies
include(FetchContent)

# 1. Download and build Abseil, a dependency of gRPC.
# The SOURCE_SUBDIR is required for FetchContent to find the correct CMakeLists.txt within the repo.
message(STATUS "🔄 Downloading Abseil...")
FetchContent_Declare(
    abseil-cpp
    GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
    GIT_TAG        20240116.2
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(abseil-cpp)

# 2. Download and build Protobuf.
# Disable tests and examples for Protobuf to avoid internal Abseil linking issues
message(STATUS "🔄 Downloading Protobuf...")
FetchContent_Declare(
    protobuf
    GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
    GIT_TAG        v26.1
    SOURCE_SUBDIR  cmake
    EXCLUDE_FROM_ALL ON
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
    CMAKE_ARGS
        -Dprotobuf_BUILD_TESTS=OFF
        -Dprotobuf_BUILD_EXAMPLES=OFF
        -Dprotobuf_USE_ZLIB=OFF
        -Dprotobuf_ABSL_PROVIDER=package
)
FetchContent_MakeAvailable(protobuf)

# 3. Download and build gRPC.
message(STATUS "🔄 Downloading gRPC...")
FetchContent_Declare(
    grpc
    GIT_REPOSITORY https://github.com/grpc/grpc.git
    GIT_TAG        v1.63.0
    SOURCE_SUBDIR  cmake
    # Correct way to handle submodules with FetchContent to avoid 'pathspec' errors.
    # The `POST_UPDATE` step executes a Git command after the initial clone.
    POST_UPDATE
        COMMAND ${CMAKE_COMMAND} -E chdir ${grpc_SOURCE_DIR}
        git submodule update --init --recursive
)
FetchContent_MakeAvailable(grpc)

get_filename_component(ROOT_DIR "${CMAKE_SOURCE_DIR}" DIRECTORY)
message(STATUS "FYP_DIR = ${ROOT_DIR}")

# Path to your .proto file (outside this folder)
set(PROTO_DIR "${ROOT_DIR}/scala/src/main/protobuf")
file(GLOB PROTO_FILES "${PROTO_DIR}/*.proto")

# Generated files output
set(GENERATED_DIR "${CMAKE_BINARY_DIR}/generated")
file(MAKE_DIRECTORY ${GENERATED_DIR})

add_custom_target(generate_protos ALL
    # Use the variable containing the executable path instead of the generator expression
    COMMAND "${PROTOC_EXECUTABLE}"
        --cpp_out=${GENERATED_DIR}
        --grpc_out=${GENERATED_DIR}
        --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN_EXECUTABLE}"
        -I ${PROTO_DIR}
        ${PROTO_FILES}
    DEPENDS
        ${PROTO_FILES}
        # Correct dependencies on the executable files themselves
        ${PROTOC_EXECUTABLE}
        ${GRPC_CPP_PLUGIN_EXECUTABLE}
    COMMENT "Generating protobuf and gRPC C++ sources"
)

# -------------------------------
# 6. Main executable
# -------------------------------
file(GLOB CPP_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_executable(hello-grpc ${CPP_FILES} ${PROTO_GENERATED_SRCS})

target_include_directories(hello-grpc PRIVATE ${GENERATED_DIR})
add_dependencies(hello-grpc generate_protos)

target_link_libraries(hello-grpc
    PRIVATE
    absl::base
    grpc::grpc++
    grpc::grpc
    protobuf::libprotobuf
)

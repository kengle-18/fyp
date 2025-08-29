cmake_minimum_required(VERSION 3.20)
project(HelloGrpc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# -------------------------------
# 1️⃣ Fetch Abseil (needed by gRPC)
# -------------------------------
FetchContent_Declare(
    abseil-cpp
    GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
    GIT_TAG 20240116.2
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(abseil-cpp)

# -------------------------------
# 2️⃣ Fetch Protobuf
# -------------------------------
FetchContent_Declare(
    protobuf
    GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
    GIT_TAG v29.3
    SOURCE_SUBDIR cmake
)
set(protobuf_BUILD_TESTS OFF)
FetchContent_MakeAvailable(protobuf)

# -------------------------------
# 3️⃣ Fetch gRPC
# -------------------------------
FetchContent_Declare(
    grpc
    GIT_REPOSITORY https://github.com/grpc/grpc.git
    GIT_TAG v1.72.1
    SOURCE_SUBDIR cmake
)
set(gRPC_BUILD_TESTS OFF)
FetchContent_MakeAvailable(grpc)
# 🔍 Debug: print all available targets after FetchContent
get_property(allTargets DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)
message(STATUS "📌 Available targets after FetchContent: ${allTargets}")



# -------------------------------
# 4️⃣ Proto and gRPC executables
# -------------------------------

# Paths to generated files
set(PROTO_DIR ${CMAKE_SOURCE_DIR}/../scala/src/main/protobuf)
set(GENERATED_DIR ${CMAKE_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_DIR})

file(GLOB PROTO_FILES "${PROTO_DIR}/*.proto")

set(PROTO_GEN_SRCS "")

set(GRPC_CPP_PLUGIN_EXECUTABLE
    "${grpc_BINARY_DIR}/${CMAKE_BUILD_TYPE}/grpc_cpp_plugin${CMAKE_EXECUTABLE_SUFFIX}"
)
message(STATUS "📌 GRPC_CPP_PLUGIN_EXECUTABLE: ${GRPC_CPP_PLUGIN_EXECUTABLE}")

# set(GRPC_CPP_PLUGIN_EXECUTABLE
#     "${grpc_BINARY_DIR}/$<CONFIG>/grpc_cpp_plugin${CMAKE_EXECUTABLE_SUFFIX}"
# )
# message(STATUS "📌 GRPC_CPP_PLUGIN_EXECUTABLE: ${GRPC_CPP_PLUGIN_EXECUTABLE}")

foreach(proto ${PROTO_FILES})
    get_filename_component(proto_name ${proto} NAME_WE)

    set(proto_srcs "${GENERATED_DIR}/${proto_name}.pb.cc")
    set(proto_hdrs "${GENERATED_DIR}/${proto_name}.pb.h")
    set(grpc_srcs "${GENERATED_DIR}/${proto_name}.grpc.pb.cc")
    set(grpc_hdrs "${GENERATED_DIR}/${proto_name}.grpc.pb.h")

    add_custom_command(
            OUTPUT ${proto_srcs} ${proto_hdrs} ${grpc_srcs} ${grpc_hdrs}
            COMMAND protoc
                -I ${PROTO_DIR}
                --cpp_out=${GENERATED_DIR}
                --grpc_out=${GENERATED_DIR}
                --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN_EXECUTABLE}
                ${proto}
            DEPENDS ${proto} protoc "${GRPC_CPP_PLUGIN_EXECUTABLE}"
            COMMENT "Generating C++ and gRPC sources from ${proto}"
            VERBATIM
        )
    list(APPEND PROTO_GEN_SRCS ${proto_srcs} ${grpc_srcs})
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

target_link_libraries(hello_grpc
    PRIVATE
    absl::base
    grpc
    grpc++
    protobuf
    libprotobuf
)

# target_link_libraries(hello_grpc
#     PRIVATE
#     grpc::grpc++
#     protobuf::libprotobuf
#     absl::base
# )
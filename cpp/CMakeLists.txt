cmake_minimum_required(VERSION 3.15)
project(HelloGrpc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

# === Fetch Protobuf ===
FetchContent_Declare(
  protobuf
  GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
  GIT_TAG v29.3 # pick a stable tag
)
set(protobuf_BUILD_TESTS OFF)
FetchContent_MakeAvailable(protobuf)

# === Fetch gRPC ===
FetchContent_Declare(
  grpc
  GIT_REPOSITORY https://github.com/grpc/grpc.git
  GIT_TAG v1.66.1 # pick a stable tag
)
set(gRPC_BUILD_TESTS OFF)
FetchContent_MakeAvailable(grpc)

# ==== Protobuf/GRPC compiler executables ====
# These are provided as IMPORTED targets from FetchContent
# Protoc binary is built inside protobuf build tree
set(PROTOC_EXECUTABLE ${protobuf_BINARY_DIR}/protoc${CMAKE_EXECUTABLE_SUFFIX})
set(GRPC_CPP_PLUGIN_EXECUTABLE ${grpc_BINARY_DIR}/grpc_cpp_plugin${CMAKE_EXECUTABLE_SUFFIX})

message(STATUS "Using protoc: ${PROTOC_EXECUTABLE}")
message(STATUS "Using grpc_cpp_plugin: ${GRPC_CPP_PLUGIN_EXECUTABLE}")

# === Proto generation ===
set(PROTO_DIR ${CMAKE_SOURCE_DIR}/../scala/src/main/protobuf)
set(GENERATED_DIR ${CMAKE_BINARY_DIR}/generated)

file(MAKE_DIRECTORY ${GENERATED_DIR})

set(PROTO_FILES
    ${PROTO_DIR}/GeminiService.proto
)

foreach(proto ${PROTO_FILES})
  get_filename_component(proto_name ${proto} NAME_WE)

  # Generated source files
  set(proto_srcs "${GENERATED_DIR}/${proto_name}.pb.cc")
  set(proto_hdrs "${GENERATED_DIR}/${proto_name}.pb.h")
  set(grpc_srcs "${GENERATED_DIR}/${proto_name}.grpc.pb.cc")
  set(grpc_hdrs "${GENERATED_DIR}/${proto_name}.grpc.pb.h")

add_custom_command(
  OUTPUT ${proto_srcs} ${proto_hdrs} ${grpc_srcs} ${grpc_hdrs}
  COMMAND ${PROTOC_EXECUTABLE}
    -I ${PROTO_DIR}
    --cpp_out=${GENERATED_DIR}
    --grpc_out=${GENERATED_DIR}
    --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN_EXECUTABLE}
    ${proto}
  DEPENDS ${proto} ${PROTOC_EXECUTABLE} ${GRPC_CPP_PLUGIN_EXECUTABLE}
  COMMENT "Generating C++ sources from ${proto}"
  VERBATIM
)


  list(APPEND PROTO_GEN_SRCS ${proto_srcs} ${grpc_srcs})
endforeach()

# === Build your gRPC target ===
add_executable(hello-grpc
  main.cpp
  ${PROTO_GEN_SRCS}
)

target_include_directories(hello-grpc PRIVATE ${GENERATED_DIR})

target_link_libraries(hello-grpc
  PRIVATE
  gRPC::grpc++
  protobuf::libprotobuf
)

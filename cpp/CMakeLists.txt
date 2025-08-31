cmake_minimum_required(VERSION 3.20)
project(HelloGrpc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Ensure all dependencies are built as static libraries
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libraries" FORCE)
set(protobuf_BUILD_PROTOC_BINARIES ON CACHE BOOL "" FORCE)

set(protobuf_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(gRPC_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

set(ABSL_ENABLE_INSTALL ON CACHE BOOL "" FORCE)
set(ABSL_PROPAGATE_CXX_STD ON CACHE BOOL "" FORCE)
set(ABSL_BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(ABSL_USE_GOOGLETEST_HEAD OFF CACHE BOOL "" FORCE)
set(absl_BUILD_SHARED_LIBS OFF)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)

set(ABSL_BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(protobuf_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(gRPC_BUILD_TESTS OFF CACHE BOOL "" FORCE)

set(ABSL_BUILD_TEST_HELPERS OFF CACHE BOOL "" FORCE)


include(FetchContent)

# -------------------------------
# 1️⃣ Fetch Abseil (needed by gRPC)
# -------------------------------

# FetchContent_Declare(
#     abseil-cpp
#     GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
#     GIT_TAG 20250127.1
# )

# set(ABSL_BUILD_TESTING OFF CACHE BOOL "" FORCE)
# set(ABSL_BUILD_TEST_HELPERS OFF CACHE BOOL "" FORCE)
# set(ABSL_USE_GOOGLETEST_HEAD OFF CACHE BOOL "" FORCE)
# set(absl_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
# FetchContent_MakeAvailable(abseil-cpp)

# -------------------------------
# 2️⃣ Fetch Protobuf
# -------------------------------
FetchContent_Declare(
    protobuf
    GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
    GIT_TAG v5.29.5
    SOURCE_SUBDIR cmake
)
set(protobuf_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(protobuf_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(protobuf_BUILD_PROTOC_BINARIES ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(protobuf)

# -------------------------------
# 3️⃣ Fetch gRPC
# -------------------------------
set(gRPC_BUILD_SHARED_LIBS OFF)
FetchContent_Declare(
    grpc
    GIT_REPOSITORY https://github.com/grpc/grpc.git
    GIT_TAG v1.71.0
    SOURCE_SUBDIR cmake
)
set(gRPC_BUILD_TESTS OFF)
FetchContent_MakeAvailable(grpc)

get_filename_component(BUILD_DEPS_DIR "${grpc_SOURCE_DIR}" PATH)
message(STATUS "📌 BUILD_DEPS_DIR: ${BUILD_DEPS_DIR}")

message(STATUS "📌 grpc_SOURCE_DIR: ${grpc_SOURCE_DIR}")
message(STATUS "📌 grpc_BINARY_DIR: ${grpc_BINARY_DIR}")


set(protobuf_SOURCE_DIR "${protobuf_SOURCE_DIR}")
message(STATUS "📌 protobuf_SOURCE_DIR: ${protobuf_SOURCE_DIR}")

set(protobuf_BINARY_DIR "${protobuf_BINARY_DIR}")
message(STATUS "📌 protobuf_BINARY_DIR: ${protobuf_BINARY_DIR}")


add_custom_target(build_protoc
    COMMAND ${CMAKE_COMMAND} 
            -S ${protobuf_SOURCE_DIR}            # source folder of protobuf
            -B ${protobuf_BINARY_DIR}            # build folder (FetchContent binary dir)
            -G "Visual Studio 17 2022"      # generator
            -A x64
            -DBUILD_SHARED_LIBS=OFF          # static build
            -DCMAKE_CXX_STANDARD=17
            -DABSL_BUILD_TESTING=OFF
            -DABSL_BUILD_TEST_HELPERS=OFF
            -DABSL_USE_GOOGLETEST_HEAD=OFF
            -Dprotobuf_BUILD_TESTS=OFF
    COMMAND ${CMAKE_COMMAND} --build ${protobuf_BINARY_DIR} --target protoc --config $<CONFIG>
    COMMENT "Building protoc executable via custom target"
)

add_custom_target(build_grpc_plugin
    COMMAND ${CMAKE_COMMAND} 
            -S ${grpc_SOURCE_DIR}            # source folder of grpc
            -B ${grpc_BINARY_DIR}            # build folder (FetchContent binary dir)
            -G "Visual Studio 17 2022"      # generator
            -A x64
            -DgRPC_BUILD_TESTS=OFF
            -DBUILD_SHARED_LIBS=OFF          # static build
            -DCMAKE_CXX_STANDARD=17
            -DABSL_BUILD_TESTING=OFF
            -DABSL_BUILD_TEST_HELPERS=OFF
            -DABSL_USE_GOOGLETEST_HEAD=OFF
    COMMAND ${CMAKE_COMMAND} --build ${grpc_BINARY_DIR} --target grpc_cpp_plugin --config $<CONFIG>
    COMMENT "Building grpc_cpp_plugin executable via custom target"
)

# find_program(PROTOC_EXECUTABLE 
#     NAMES protoc protoc.exe
#     PATHS 
#         # "${PROTOBUF_DEST_EXE_DIR}/Release"
#         # "${PROTOBUF_DEST_EXE_DIR}/Debug" 
#         # "${PROTOBUF_DEST_EXE_DIR}"
#         # "${PROTOBUF_SRC_EXE_DIR}/Release"
#         # "${PROTOBUF_SRC_EXE_DIR}/Debug"
#         "C:/Users/user/Downloads/fyp/cpp/build/_deps/grpc-build/third_party/protobuf/Release"
#     NO_DEFAULT_PATH
# )

set(PROTOC_EXECUTABLE "${grpc_BINARY_DIR}/third_party/protobuf/$<CONFIG>/protoc.exe")
message(INFO "PROTOC_EXECUTABLE found at: ${PROTOC_EXECUTABLE}")

set(GRPC_CPP_PLUGIN_EXECUTABLE "${grpc_BINARY_DIR}/$<CONFIG>/grpc_cpp_plugin.exe")
message(INFO "GRPC_CPP_PLUGIN_EXECUTABLE found at: ${GRPC_CPP_PLUGIN_EXECUTABLE}")

if(NOT EXISTS "${PROTOC_EXECUTABLE}")
    message(WARNING "protoc executable not found at: ${PROTOC_EXECUTABLE}")
endif()


if(NOT EXISTS "${GRPC_CPP_PLUGIN_EXECUTABLE}")
    message(WARNING "grpc_cpp_plugin executable not found at: ${GRPC_CPP_PLUGIN_EXECUTABLE}")
endif()


# -------------------------------
# 4️⃣ Proto and gRPC executables
# -------------------------------

# Paths to generated files
get_filename_component(ROOT_DIR "${CMAKE_SOURCE_DIR}" PATH)
message(STATUS "ROOT_DIR: ${ROOT_DIR}")
set(PROTO_DIR "${ROOT_DIR}/scala/src/main/protobuf")
message(STATUS "📌 APROTO_DIR: ${PROTO_DIR}")
set(GENERATED_DIR ${CMAKE_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_DIR})

file(GLOB PROTO_FILES "${PROTO_DIR}/*.proto")

set(PROTO_GEN_SRCS "")

foreach(proto ${PROTO_FILES})
    get_filename_component(proto_name ${proto} NAME_WE)

    message(STATUS "📌 ${GENERATED_DIR}/${proto_name}.pb.cc: ${${GENERATED_DIR}/${proto_name}.pb.cc}")

    set(proto_srcs "${GENERATED_DIR}/${proto_name}.pb.cc")
    set(proto_hdrs "${GENERATED_DIR}/${proto_name}.pb.h")
    set(grpc_srcs "${GENERATED_DIR}/${proto_name}.grpc.pb.cc")
    set(grpc_hdrs "${GENERATED_DIR}/${proto_name}.grpc.pb.h")

add_custom_command(
    OUTPUT ${proto_srcs} ${proto_hdrs} ${grpc_srcs} ${grpc_hdrs}
    COMMAND ${CMAKE_COMMAND} -E echo "Running protoc on ${proto}"
    COMMAND ${CMAKE_COMMAND} -E echo "  Using PROTOC: ${PROTOC_EXECUTABLE}"
    COMMAND ${CMAKE_COMMAND} -E echo "  Using GRPC plugin: ${GRPC_CPP_PLUGIN_EXECUTABLE}"
    COMMAND ${PROTOC_EXECUTABLE}
        -I ${PROTO_DIR}
        --cpp_out=${GENERATED_DIR}
        --grpc_out=${GENERATED_DIR}
        --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN_EXECUTABLE}
        ${proto}
    DEPENDS ${proto}
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

add_dependencies(hello_grpc build_protoc build_grpc_plugin)

message(STATUS "📌 Final")
target_include_directories(hello_grpc 
    PRIVATE 
    ${GENERATED_DIR} 
)

add_library(protobuf_lib STATIC IMPORTED)

# set_target_properties(protobuf_lib PROPERTIES
#     IMPORTED_LOCATION "${grpc_BINARY_DIR}/third_party/protobuf/Release/libprotobuf.lib"
#     INTERFACE_INCLUDE_DIRECTORIES "${grpc_SOURCE_DIR}/third_party/protobuf/src"
# )

target_link_libraries(hello_grpc
    PRIVATE
    grpc++
    protobuf
)

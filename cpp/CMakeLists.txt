cmake_minimum_required(VERSION 3.20)
project(HelloGrpc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Ensure all dependencies are built as static libraries
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libraries" FORCE)
set(protobuf_BUILD_PROTOC_BINARIES ON)

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

set(protobuf_BUILD_PROTOC_BINARIES ON) 

include(FetchContent)

# -------------------------------
# 1️⃣ Fetch Abseil (needed by gRPC)
# -------------------------------
FetchContent_Declare(
    abseil-cpp
    GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
    GIT_TAG 20250127.1
)
FetchContent_MakeAvailable(abseil-cpp)

# -------------------------------
# 2️⃣ Fetch Protobuf
# -------------------------------
set(protobuf_BUILD_SHARED_LIBS OFF)
FetchContent_Declare(
    protobuf
    GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
    GIT_TAG v5.29.5
    SOURCE_SUBDIR cmake
)
set(protobuf_BUILD_TESTS OFF)
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

add_custom_target(build_protoc
    COMMAND ${CMAKE_COMMAND} 
            -S ${protobuf_SOURCE_DIR} 
            -B ${protobuf_BINARY_DIR} 
            -G "Visual Studio 17 2022" 
            -A x64
            -Dprotobuf_BUILD_TESTS=OFF
            -DBUILD_SHARED_LIBS=OFF
            -DCMAKE_CXX_STANDARD=17
            -DABSL_BUILD_TESTING=OFF
            -DABSL_BUILD_TEST_HELPERS=OFF
            -DABSL_USE_GOOGLETEST_HEAD=OFF
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

set(PROTOC_EXECUTABLE "${protobuf_BINARY_DIR}/$<CONFIG>/protoc${CMAKE_EXECUTABLE_SUFFIX}")
set(GRPC_CPP_PLUGIN_EXECUTABLE "${grpc_BINARY_DIR}/$<CONFIG>/grpc_cpp_plugin${CMAKE_EXECUTABLE_SUFFIX}")


# 🔍 Debug: print all available targets after FetchContent
get_property(allTargets DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)
message(STATUS "📌 Available targets after FetchContent: ${allTargets}")


# Ensure protoc and grpc_cpp_plugin are built
# set(PROTOC_EXECUTABLE $<TARGET_FILE:protoc>)
# set(GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:grpc_cpp_plugin>)

# message(STATUS "PROTOC_EXECUTABLE: ${PROTOC_EXECUTABLE}")
# message(STATUS "GRPC_CPP_PLUGIN_EXECUTABLE: ${GRPC_CPP_PLUGIN_EXECUTABLE}")

# find_program(PROTOC_EXECUTABLE protoc HINTS ${protobuf_BINARY_DIR})
# find_program(GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin HINTS ${grpc_BINARY_DIR})

# message(STATUS "Using protoc: ${PROTOC_EXECUTABLE} : ${protobuf_BINARY_DIR}")
# message(STATUS "Using grpc_cpp_plugin: ${GRPC_CPP_PLUGIN_EXECUTABLE}")


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

# set(GRPC_CPP_PLUGIN_EXECUTABLE
#     "${CMAKE_BINARY_DIR}/_deps/grpc-build/Release/grpc_cpp_plugin.exe"
# )

set(GRPC_CPP_PLUGIN_EXECUTABLE
    "${CMAKE_BINARY_DIR}/_deps/grpc-build/Release/grpc_cpp_plugin.exe"
)

set(PROTOC_EXECUTABLE
    "${CMAKE_BINARY_DIR}/_deps/protobuf-build/Release/protoc.exe"
)

# message(STATUS "📌 GRPC_CPP_PLUGIN_EXECUTABLE: ${GRPC_CPP_PLUGIN_EXECUTABLE}")
# if(NOT EXISTS "${GRPC_CPP_PLUGIN_EXECUTABLE}")
#     message(FATAL_ERROR "grpc_cpp_plugin not found at: ${GRPC_CPP_PLUGIN_EXECUTABLE}")
# endif()

# set(GRPC_CPP_PLUGIN_EXECUTABLE
#     "${grpc_BINARY_DIR}/$<CONFIG>/grpc_cpp_plugin${CMAKE_EXECUTABLE_SUFFIX}"
# )
# message(STATUS "📌 GRPC_CPP_PLUGIN_EXECUTABLE: ${GRPC_CPP_PLUGIN_EXECUTABLE}")

# add_custom_target(build_protoc
#     COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/_deps/protobuf-build --target protoc --config Release
#     COMMENT "Building protoc executable"
# )

# # Build grpc_cpp_plugin
# add_custom_target(build_grpc_plugin
#     COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/_deps/grpc-build --target grpc_cpp_plugin --config Release
#     COMMENT "Building grpc_cpp_plugin executable"
# )

foreach(proto ${PROTO_FILES})
    get_filename_component(proto_name ${proto} NAME_WE)

    message(STATUS "📌 ${GENERATED_DIR}/${proto_name}.pb.cc: ${${GENERATED_DIR}/${proto_name}.pb.cc}")

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
            DEPENDS ${proto} build_protoc build_grpc_plugin
            COMMENT "Generating C++ and gRPC sources from ${proto}"
            VERBATIM
        )

    # add_custom_command(
    #     OUTPUT ${proto_srcs} ${proto_hdrs} ${grpc_srcs} ${grpc_hdrs}
    #     COMMAND $<TARGET_FILE:protobuf::protoc>
    #         -I ${PROTO_DIR}
    #         --cpp_out=${GENERATED_DIR}
    #         --grpc_out=${GENERATED_DIR}
    #         --plugin=protoc-gen-grpc=$<TARGET_FILE:grpc::grpc_cpp_plugin>
    #         ${proto}
    #     DEPENDS ${proto}
    #     COMMENT "Generating C++ and gRPC sources from ${proto}"
    #     VERBATIM
    # )
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

message(STATUS "📌 Final")
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

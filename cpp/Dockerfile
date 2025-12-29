# syntax=docker/dockerfile:1.4
# ===========================
# 1. Build stage
# ===========================
FROM ubuntu:22.04 AS builder

# Install build dependencies + system gRPC/Protobuf
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    git \
    pkg-config \
    libssl-dev \
    ccache \
    ca-certificates \
    libgrpc++-dev \
    libprotobuf-dev \
    protobuf-compiler \
    protobuf-compiler-grpc \
    && rm -rf /var/lib/apt/lists/*


WORKDIR /app

# Copy CMake first for caching
COPY CMakeLists.txt .

# Copy source and proto files
COPY src ./src

RUN mkdir -p src/generated && \
    protoc -I=src/protobuf \
           --cpp_out=src/generated \
           --grpc_out=src/generated \
           --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` \
           --experimental_allow_proto3_optional \
           src/protobuf/base.proto


# Pre-create build directory and configure
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Build the application using ccache for faster incremental builds
RUN --mount=type=cache,target=/root/.ccache \
    cmake --build build --config Release -j$(nproc)

# For debug 
FROM ubuntu:22.04 AS generate

RUN apt-get update && apt-get install -y --no-install-recommends \
    protobuf-compiler \
    protobuf-compiler-grpc \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY CMakeLists.txt .

# Copy source and proto files
COPY src/protobuf ./src/protobuf

RUN mkdir -p src/generated && \
    protoc -I=src/protobuf \
           --cpp_out=src/generated \
           --grpc_out=src/generated \
           --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` \
           --experimental_allow_proto3_optional \
           src/protobuf/base.proto

# ===========================
# 2. Runtime stage
# ===========================
FROM ubuntu:22.04 AS runtime

# Install only runtime libs
RUN apt-get update && apt-get install -y --no-install-recommends \
    libstdc++6 \
    libgrpc++-dev \
    libprotobuf-dev \
    protobuf-compiler \
    tcpdump \
    netcat \
    && rm -rf /var/lib/apt/lists/*

# Create non-root user
RUN useradd -m -u 1000 appuser
# Copy only final executable
COPY --from=builder /app/build/hello_grpc /app/hello_grpc
COPY --from=builder /app/build/server /app/server
COPY --from=builder /app/build/client /app/client

# Permissions
RUN chmod +x /app/hello_grpc /app/server /app/client && \
    chown appuser:appuser /app/hello_grpc /app/server /app/client

# Optional: add /app to PATH so you can run "server" instead of "/app/server"
ENV PATH="/app:$PATH"

USER appuser
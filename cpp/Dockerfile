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

# Pre-create build directory and configure
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Build the application using ccache for faster incremental builds
RUN --mount=type=cache,target=/root/.ccache \
    cmake --build build --config Release -j$(nproc)

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
    && rm -rf /var/lib/apt/lists/*

# Create non-root user
RUN useradd -m -u 1000 appuser

# Copy only final executable
COPY --from=builder /app/build/hello_grpc /usr/local/bin/hello_grpc

# Permissions
RUN chmod +x /usr/local/bin/hello_grpc

USER appuser

# Entrypoint
ENTRYPOINT ["hello_grpc"]

# Multi-stage Docker build for gRPC C++ application
FROM ubuntu:22.04 AS builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    libssl-dev \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy CMakeLists.txt first for better layer caching
COPY CMakeLists.txt .
COPY src ./src

# Create build directory and configure for Linux
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Copy source code
COPY . .

# Build the application
RUN cmake --build build --config Release -j$(nproc)

# Runtime stage - much smaller final image
FROM ubuntu:22.04 AS runtime

# Install only runtime dependencies if needed
RUN apt-get update && apt-get install -y \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

# Create non-root user for security
RUN useradd -m -u 1000 appuser

# Copy only the built executable
COPY --from=builder /app/build/hello_grpc /usr/local/bin/hello_grpc

# Set ownership and make executable
RUN chmod +x /usr/local/bin/hello_grpc

# Switch to non-root user
USER appuser

# Set the entrypoint
ENTRYPOINT ["hello_grpc"]
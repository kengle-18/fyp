# Stage 1: Build Environment
# Use a base image with a C++ compiler (e.g., GCC)
FROM gcc:12 AS builder

# Install cmake and other necessary build tools
RUN apt-get update && apt-get install -y cmake

# Set the working directory
WORKDIR /app

# Copy the source code and CMake build file
COPY main.cpp .
COPY CMakeLists.txt .

# Compile the application using CMake
RUN cmake . -DCMAKE_EXE_LINKER_FLAGS=-static
RUN make

# Stage 2: Final Runtime Image
# Use a minimal base image that is self-contained
FROM alpine:latest

# Set the working directory
WORKDIR /app

# Copy the compiled executable from the builder stage
# The name `hello-cpp` is defined in CMakeLists.txt
COPY --from=builder /app/hello-cpp .

# The command to run the executable when the container starts
CMD ["./hello-cpp"]
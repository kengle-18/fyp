#pragma once

// ===========================
// Common Includes
// ===========================

// Standard C++ headers
#include <cstdlib>       // std::getenv
#include <iostream>      // std::cout, std::cerr
#include <memory>        // std::unique_ptr, std::shared_ptr
#include <string>        // std::string
#include <thread>        // std::thread (optional, for multi-threaded use)
#include <chrono>        // std::chrono::seconds, etc.
#include <fstream>       // std::ofstream
#include <filesystem>    // std::filesystem (C++17)
#include <map>           // std::map
#include <csignal>       // for system signal

// gRPC / Protobuf
#include <grpcpp/grpcpp.h>   // Main gRPC API
#include <grpcpp/server_builder.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/security/credentials.h>

// Generated Protobuf & gRPC headers
// (adjust the path to match your generated folder)
#include "generated/all_generated.h"

// ===========================
// Common Namespace Shortcuts
// ===========================

using grpc::Channel;
using grpc::ClientContext;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using com::example::Greeter;
using com::example::HelloRequest;
using com::example::HelloReply;

using com::example::UniversalMessage;
using com::example::UniversalTester;

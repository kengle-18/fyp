#pragma once

#include "common.h"      // gRPC and protobuf includes
#include "grpcHelper.h"  // your GreeterServiceImpl declaration

using grpc::Server;
using grpc::ServerBuilder;

// Function to start the server
void RunServer(const std::string& server_address);

// Global server pointer for signal handling
inline std::unique_ptr<Server> server;
// Signal handler for Ctrl+C / Docker stop
inline void HandleSignal(int signum) {
    std::cout << "\n⚡ Received signal " << signum << ", shutting down server immediately..." << std::endl;
    if (server) {
        // Immediate shutdown: cancel all ongoing RPCs
        server->Shutdown(std::chrono::system_clock::now());
    }
    std::exit(0); // Exit process
}

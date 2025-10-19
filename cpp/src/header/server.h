#pragma once

#include "common.h"      // gRPC and protobuf includes
#include "grpcHelper.h"  // your GreeterServiceImpl declaration

using grpc::Server;
using grpc::ServerBuilder;

// Function to start the server
void RunServer(const std::string& server_address);

#pragma once

// ===================================
// Common includes
// ===================================
#include "header/common.h"

// ===================================
// GreeterServiceImpl (Server-side)
// ===================================
class GreeterServiceImpl final : public Greeter::Service {
public:
    // Implementation of the RPC defined in your proto
    grpc::Status SayHello(grpc::ServerContext* context, const HelloRequest* request,
                          HelloReply* reply) override;
};

// ===================================
// GreeterClient (Client-side)
// ===================================
class GreeterClient {
public:
    explicit GreeterClient(std::shared_ptr<Channel> channel);
    std::string SayHello(const std::string& name);

private:
    std::unique_ptr<Greeter::Stub> stub_;
};

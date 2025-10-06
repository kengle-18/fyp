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
    Status SayHello(ServerContext* context, const HelloRequest* request,
                    HelloReply* reply) override {
        reply->set_message("Hello, " + request->name() + "!");
        return Status::OK;
    }
};

// ===================================
// GreeterClient (Client-side)
// ===================================
class GreeterClient {
public:
    explicit GreeterClient(std::shared_ptr<Channel> channel)
        : stub_(Greeter::NewStub(channel)) {}

    std::string SayHello(const std::string& name) {
        HelloRequest request;
        request.set_name(name);

        HelloReply reply;
        ClientContext context;

        Status status = stub_->SayHello(&context, request, &reply);
        if (status.ok()) {
            return reply.message();
        } else {
            return "RPC failed: " + status.error_message();
        }
    }

private:
    std::unique_ptr<Greeter::Stub> stub_;
};

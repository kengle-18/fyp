#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "generated/all_generated.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using com::example::Greeter;
using com::example::HelloRequest;
using com::example::HelloReply;

class GreeterServiceImpl final : public Greeter::Service {
public:
    Status SayHello(ServerContext* context, const HelloRequest* request,
                    HelloReply* reply) override {
        reply->set_message("Hello, " + request->name() + "!");
        return Status::OK;
    }
};

int main(int argc, char** argv) {
    std::string server_address("0.0.0.0:50051");
    GreeterServiceImpl service;

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "✅ Server listening on " << server_address << std::endl;

    server->Wait();
    return 0;
}

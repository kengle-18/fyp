#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <chrono>

#include <grpcpp/grpcpp.h>
#include "generated/all_generated.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using com::example::Greeter;
using com::example::HelloRequest;
using com::example::HelloReply;

// ===================== SERVER =====================
class GreeterServiceImpl final : public Greeter::Service {
public:
    Status SayHello(ServerContext* context, const HelloRequest* request,
                    HelloReply* reply) override {
        reply->set_message("Hello, " + request->name() + "!");
        return Status::OK;
    }
};

void RunServer() {
    std::string server_address("0.0.0.0:50051");
    GreeterServiceImpl service;

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "✅ Server listening on " << server_address << std::endl;

    // Run server loop (blocking)
    server->Wait();
}

// ===================== CLIENT =====================
class GreeterClient {
public:
    GreeterClient(std::shared_ptr<Channel> channel)
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

// ===================== MAIN =====================
int main(int argc, char** argv) {
    // Start server in separate thread
    std::thread server_thread(RunServer);

    // Give server time to start
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Create client and call SayHello
    GreeterClient client(grpc::CreateChannel("localhost:50051",
                                             grpc::InsecureChannelCredentials()));

    std::string user("DockerUser");
    std::string reply = client.SayHello(user);
    std::cout << "👋 Client received: " << reply << std::endl;

    // Stop server gracefully (in real apps, use signals/shutdown)
    server_thread.detach(); // let server run forever
    return 0;
}

#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "generated/all_generated.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using com::example::Greeter;
using com::example::HelloRequest;
using com::example::HelloReply;

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

int main(int argc, char** argv) {
    // Default values (can be overridden by environment variables)
    const char* host_env = std::getenv("SERVER_HOST");
    const char* port_env = std::getenv("SERVER_PORT");

    std::string host = host_env ? host_env : "localhost";
    std::string port = port_env ? port_env : "50051";
    std::string server_address = host + ":" + port;

    GreeterClient client(grpc::CreateChannel(
        server_address, grpc::InsecureChannelCredentials()));

    std::string user("DockerUserTesting");
    std::string reply = client.SayHello(user);

    std::cout << "👋 Client received: " << reply << std::endl;
    return 0;
}

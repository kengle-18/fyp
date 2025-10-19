#include "../header/grpcHelper.h"

// ===================================
// GreeterServiceImpl (Server-side)
// ===================================
grpc::Status GreeterServiceImpl::SayHello(grpc::ServerContext* context, const HelloRequest* request,
                                          HelloReply* reply) {
    // Create file to test 
    std::ofstream serverFile("/app/cpp/src/server.txt", std::ios::trunc);
    if (serverFile.is_open()) {
        serverFile << R"(Hello from the server side!)";
        serverFile.close();
    }
    reply->set_message("Hello, " + request->name() + "!");
    return grpc::Status::OK;
}

// ===================================
// GreeterClient (Client-side)
// ===================================
GreeterClient::GreeterClient(std::shared_ptr<grpc::Channel> channel)
    : stub_(Greeter::NewStub(channel)) {}

std::string GreeterClient::SayHello(const std::string& name) {
    HelloRequest request;
    request.set_name(name);

    HelloReply reply;
    grpc::ClientContext context;

    grpc::Status status = stub_->SayHello(&context, request, &reply);
    if (status.ok()) {
        std::ofstream serverFile("/app/cpp/src/client.txt", std::ios::trunc);
        if (serverFile.is_open()) {
            serverFile << R"(Hello from the client side!)";
            serverFile.close();
        }
        return reply.message();
    } else {
        return "RPC failed: " + status.error_message();
    }
}

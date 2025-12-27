#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <chrono>

#include <grpcpp/grpcpp.h>
#include "generated/all_generated.h"

using com::example::Greeter;
using com::example::HelloRequest;
using com::example::HelloReply;

#include "../header/common.h"

// ===================== SERVER =====================
class GreeterServiceImpl final : public Greeter::Service {
public:
    Status SayHello(ServerContext* context, const HelloRequest* request,
                    HelloReply* reply) override {
        reply->set_message("Hello, " + request->name() + "!");
        return Status::OK;
    }
};

class UniversalTesterImpl final : public UniversalTester::Service {
public:
    grpc::Status SendUniversal(grpc::ServerContext* context,
                               const UniversalMessage* request,
                               UniversalMessage* response) override {
        std::cout << "🧪 Received UniversalMessage:\n"
                  << request->DebugString() << std::endl;

        // Echo the message back with a small modification
        UniversalMessage reply_msg = *request;

        if (request->has_single_int()) {
            int value = request->single_int();
            reply_msg.set_single_int(value + 1);  // example mutation
            std::cout << "🔢 Incremented single_int: " << value << " → "
                      << reply_msg.single_int() << std::endl;
        } else {
            reply_msg.set_single_int(100);  // set a default if missing
        }

        *response = reply_msg;
        return grpc::Status::OK;
    }
};



void RunServer() {
    std::string server_address("0.0.0.0:50051");
    GreeterServiceImpl service;
    UniversalTesterImpl service2;

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    builder.RegisterService(&service2);
    std::cout << "✅ UniversalTester service registered\n";

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


class UniversalTesterClient {
public:
    explicit UniversalTesterClient(std::shared_ptr<grpc::Channel> channel)
        : stub_(UniversalTester::NewStub(channel)) {}

    UniversalMessage SendUniversal(const UniversalMessage& msg) {
        UniversalMessage reply;
        grpc::ClientContext context;

        grpc::Status status = stub_->SendUniversal(&context, msg, &reply);
        if (!status.ok()) {
            std::cerr << "❌ RPC failed: " << status.error_message() << std::endl;
        }
        return reply;
    }

private:
    std::unique_ptr<UniversalTester::Stub> stub_;
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

    UniversalTesterClient tester(grpc::CreateChannel("localhost:50051",
        grpc::InsecureChannelCredentials()));

    std::string user("DockerUserMains");
    std::string reply = client.SayHello(user);
    std::cout << "👋 Client received: " << reply << std::endl;

    UniversalMessage msg;
    if (!msg.has_single_int()) {
        std::cout << "singleint is not set yet\n";
    }

    // Set the value
    msg.set_single_int(43);

    // Check again
    if (msg.has_single_int()) {
        std::cout << "singleint is set to: " << msg.single_int() << "\n";
    }

    UniversalMessage reply2 = tester.SendUniversal(msg);
    std::cout << "📥 Reply from UniversalTester:\n" << reply2.DebugString() << std::endl;
    // msg.set_optionalsingleint(42);
    // msg.set_optionalsinglebool(true);
    // std::cout << "msg is: " << msg.DebugString() << std::endl;
    // client.SendAllMessages(msg);

    // Stop server gracefully (in real apps, use signals/shutdown)
    server_thread.detach(); // let server run forever
    return 0;
}

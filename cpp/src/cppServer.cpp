#include "header/common.h"
#include "header/grpcHelper.h"

int main(int argc, char** argv) {
    const char* port_env = std::getenv("SERVER_PORT");
    std::string port = port_env ? port_env : "50051";
    std::string server_address = "0.0.0.0:" + port;
    GreeterServiceImpl service;

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "✅ Server listening on " << server_address << std::endl;

    server->Wait();
    return 0;
}

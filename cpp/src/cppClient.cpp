#include "header/common.h"
#include "header/grpcHelper.h"

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

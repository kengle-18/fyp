#include "header/common.h"
#include "header/grpcHelper.h"
#include "header/config.h"

int main(int argc, char** argv) {
    // Default values (can be overridden by environment variables)
    const char* host_env = std::getenv("SERVER_HOST");
    const char* port_env = std::getenv("SERVER_PORT");

    std::string host = host_env ? host_env : "localhost";
    std::string port = port_env ? port_env : "50051";
    std::string server_address = host + ":" + port;

    GreeterClient client(grpc::CreateChannel(
        server_address, grpc::InsecureChannelCredentials()));

    Config config(argc, argv);
    std::string name = config.getArg(0, "DefaultName");
    std::string option = config.getArg(1, "DefaultOption");

    std::cout << "📘 Name selected: " << name << std::endl;
    std::cout << "📘 Option selected: " << option << std::endl;

    auto allArgs = config.getAll({"Default", "Default"});
    std::ostringstream merged;
    for (size_t i = 0; i < allArgs.size(); ++i) {
        merged << allArgs[i];
        if (i + 1 < allArgs.size()) merged << ", ";
    }
    std::cout << "📘 All args merged: " << merged.str() << std::endl;

    std::string user("DockerUserTesting");
    std::string reply = client.SayHello(user);

    std::cout << "👋 Client received: " << reply << std::endl;
    return 0;
}

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

    GrpcClient client(grpc::CreateChannel(
        server_address, grpc::InsecureChannelCredentials()));

    Config config(argc, argv);

    auto allArgs = config.getAllArgsInLine();
    UniversalMessage msg;

    // std::cout << "Command-line Arguments:\n";
    for (size_t line = 0; line < allArgs.size(); ++line) {
        std::cout << "Line " << line << ": " << std::endl;
        client.setFieldsWithConfigValues(allArgs[line], msg);
        client.SendAllMessages(msg);
    }

    return 0;
}

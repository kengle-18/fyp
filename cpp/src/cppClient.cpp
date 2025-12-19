#include "header/common.h"
#include "header/grpcHelper.h"
#include "header/config.h"
#include "header/io.h"

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

    const char* env = std::getenv("OUTPUT_DIR_GENERATED");
    const char* fileEnv1 = std::getenv("OUTPUT_FILE_CPP_MESSAGE_INITAL");
    const char* fileEnv2 = std::getenv("OUTPUT_FILE_CPP_MESSAGE_INDIVIDUAL_FIELD");
    const char* fileEnv3 = std::getenv("OUTPUT_FILE_CPP_RESPONSE_FROM_SERVER");

    std::filesystem::path outputFile1 = fileEnv1 ? std::filesystem::path(fileEnv1) : std::filesystem::path("default.txt");
    std::filesystem::path outputFile2 = fileEnv2 ? std::filesystem::path(fileEnv2) : std::filesystem::path("default.txt");
    std::filesystem::path outputFile3 = fileEnv3 ? std::filesystem::path(fileEnv3) : std::filesystem::path("default.txt");

    std::filesystem::path cppMessageInital = std::filesystem::path(env ? env : ".") / outputFile1;
    std::filesystem::path cppMessageIndiviualField = std::filesystem::path(env ? env : ".") / outputFile2;
    std::filesystem::path cppResponseFromServer = std::filesystem::path(env ? env : ".") / outputFile3;
    

    auto allArgs = config.getAllArgsInLine();
    UniversalMessage msg;

    // std::cout << "Command-line Arguments:\n";
    for (size_t line = 0; line < allArgs.size(); ++line) {
        // std::cout << "Line " << line << ": " << std::endl;
        client.setFieldsWithConfigValues(allArgs[line], msg);
        // Make set one line send one line message
    }

    FileUtils::writeToFile(cppMessageInital.string(), msg.DebugString());

    // Send all fields then individually
    client.SendAllMessages(msg, cppMessageIndiviualField, cppResponseFromServer);

    return 0;
}

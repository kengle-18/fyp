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
    }

    //set all the fields manually

    // ---------- Scalar fields ----------
    // msg.set_single_int(7);
    // msg.set_single_int(42);
    // msg.set_big_int(123456789L);
    // msg.set_single_string("hello");
    // msg.set_single_bool(true);
    // msg.set_single_double(3.14);
    // msg.set_single_float(2.718f);
    // msg.set_single_bytes("bytes");  // std::string is OK

    // ---------- Repeated fields ----------
    // msg.add_repeated_int(1);
    // msg.add_repeated_int(2);

    // msg.add_repeated_string("foo");
    // msg.add_repeated_bool(true);
    // msg.add_repeated_double(1.1);
    // msg.add_repeated_float(2.2f);
    // msg.add_repeated_big_int(1000000000L);
    // msg.add_repeated_bytes("bar");

    // ---------- Map fields ----------
    // Maps behave like std::map, Proto generates map-like API
    // (*msg.mutable_map_int_string())[1] = "one";
    // (*msg.mutable_map_string_int())["a"] = 100;

    auto& nestedMap = (*msg.mutable_map_int_nested())[1];
    nestedMap.set_name("nested1");
    nestedMap.set_value(10);

    // ---------- Nested ----------
    auto* nested = msg.mutable_nested();
    nested->set_name("top");
    nested->set_value(99);

    // ---------- Repeated nested ----------
    auto* r1 = msg.add_repeated_nested();
    r1->set_name("r1");

    auto* r2 = msg.add_repeated_nested();
    r2->set_name("r2");
    r2->set_value(2);

    // ---------- Enum ----------
    msg.set_status(UniversalMessage::ACTIVE);
    msg.add_repeated_status(UniversalMessage::INACTIVE);

    // std::cout<< msg.DebugString() << std::endl;

    FileUtils::writeToFile(cppMessageInital.string(), msg.DebugString());

    // Send all fields then individually
    client.SendAllMessages(msg, cppMessageIndiviualField, cppResponseFromServer);

    return 0;
}

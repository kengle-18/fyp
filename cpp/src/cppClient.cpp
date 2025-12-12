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

    const char* env = std::getenv("OUTPUT_DIR_GENERATED");
    std::filesystem::path cppMessageInital = std::filesystem::path(env ? env : ".") / "cppMessageInital.txt";
    std::filesystem::path cppMessageIndiviualField = std::filesystem::path(env ? env : ".") / "cppMessageIndiviualField.txt";
    std::filesystem::path cppResponseFromServer = std::filesystem::path(env ? env : ".") / "cppResponseFromServer.txt";
    

    UniversalMessage msg;

    //set all the fields manually

    // ---------- Scalar fields ----------
    msg.set_single_int(42);
    msg.set_big_int(123456789L);
    msg.set_single_string("hello");
    msg.set_single_bool(true);
    msg.set_single_double(3.14);
    msg.set_single_float(2.718f);
    msg.set_single_bytes("bytes");  // std::string is OK

    // ---------- Repeated fields ----------
    msg.add_repeated_int(1);
    msg.add_repeated_int(2);

    msg.add_repeated_string("foo");
    msg.add_repeated_bool(true);
    msg.add_repeated_double(1.1);
    msg.add_repeated_float(2.2f);
    msg.add_repeated_big_int(1000000000L);
    msg.add_repeated_bytes("bar");

    // ---------- Map fields ----------
    // Maps behave like std::map, Proto generates map-like API
    (*msg.mutable_map_int_string())[1] = "one";
    (*msg.mutable_map_string_int())["a"] = 100;

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

    FileUtils::writeToFile(cppMessageInital.string(), msg.DebugString());

    // Send all fields then individually
    client.SendAllMessages(msg, cppMessageIndiviualField, cppResponseFromServer);

    return 0;
}

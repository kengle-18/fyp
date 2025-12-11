#include "../header/server.h"

void RunServer(const std::string& server_address) {
    // Create service
    UniversalTesterImpl service;

    // Build server
    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "✅ Server listening on " << server_address << std::endl;

    // Blocking wait
    server->Wait();
}

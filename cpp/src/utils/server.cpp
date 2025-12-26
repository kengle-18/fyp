#include "../header/server.h"
#include "../header/captureAllHeadersInterceptor.h"

void RunServer(const std::string& server_address) {
    // Create service
    UniversalTesterImpl service;

    // Build server
    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::vector<std::unique_ptr<grpc::experimental::ServerInterceptorFactoryInterface>> 
        interceptor_factories;
    interceptor_factories.push_back(
        std::make_unique<CaptureAllHeadersInterceptorFactory>());
    builder.experimental().SetInterceptorCreators(std::move(interceptor_factories));

    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "✅ Server listening on " << server_address << std::endl;

    // Blocking wait
    server->Wait();
}

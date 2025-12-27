#include "../header/common.h"
#include "../header/captureAllHeadersInterceptor.h"
#include <grpcpp/server_context.h>



CaptureAllHeadersInterceptor::CaptureAllHeadersInterceptor(grpc::experimental::ServerRpcInfo* info) {
    (void) info; // silence unsed warning 
}

void CaptureAllHeadersInterceptor::Intercept(grpc::experimental::InterceptorBatchMethods* methods) {
    if (methods->QueryInterceptionHookPoint(
            grpc::experimental::InterceptionHookPoints::POST_RECV_INITIAL_METADATA)) {
        
        std::cout << "=== ALL INCOMING HEADERS ===" << std::endl;
        
        // Get metadata from context
        auto* metadata = methods->GetRecvInitialMetadata();
        std::map<std::string, std::string> all_headers;
        
        for (const auto& pair : *metadata) {
            std::string key(pair.first.data(), pair.first.size());
            std::string value(pair.second.data(), pair.second.size());
            all_headers[key] = value;
            std::cout << key << " -> " << value << std::endl;
        }
        std::cout << "Total headers: " << all_headers.size() << std::endl;
        
        // Store headers in context for service access
        // (you'll access this in your service implementation)
    }
    methods->Proceed();
}

grpc::experimental::Interceptor* CaptureAllHeadersInterceptorFactory::CreateServerInterceptor(
    grpc::experimental::ServerRpcInfo* info) {
    return new CaptureAllHeadersInterceptor(info);
}

#pragma once
#include "header/common.h"
#include <grpcpp/impl/codegen/interceptor.h>

namespace grpc {
namespace experimental {
class ServerRpcInfo;
class InterceptorBatchMethods;
}
}

class CaptureAllHeadersInterceptor : public grpc::experimental::Interceptor {
public:
    explicit CaptureAllHeadersInterceptor(grpc::experimental::ServerRpcInfo* info);

    void Intercept(grpc::experimental::InterceptorBatchMethods* methods) override;
};

class CaptureAllHeadersInterceptorFactory
    : public grpc::experimental::ServerInterceptorFactoryInterface {
public:
    grpc::experimental::Interceptor* CreateServerInterceptor(
        grpc::experimental::ServerRpcInfo* info) override;
};
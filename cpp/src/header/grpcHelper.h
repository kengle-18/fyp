#pragma once

// ===================================
// Common includes
// ===================================
#include "header/common.h"

// ===================================
// GreeterServiceImpl (Server-side)
// ===================================
class GreeterServiceImpl final : public Greeter::Service {
public:
    // Implementation of the RPC defined in your proto
    grpc::Status SayHello(grpc::ServerContext* context, const HelloRequest* request,
                          HelloReply* reply) override;
};

class UniversalTesterImpl final : public UniversalTester::Service {
public:
    grpc::Status SendUniversal(grpc::ServerContext* context,
                               const UniversalMessage* request,
                               UniversalMessage* response) override;
};

// ===================================
// GreeterClient (Client-side)
// ===================================
class GreeterClient {
public:
    explicit GreeterClient(std::shared_ptr<Channel> channel);
    std::string SayHello(const std::string& name);

private:
    std::unique_ptr<Greeter::Stub> stub_;
};

// ===================================
class GrpcClient {
public:
    explicit GrpcClient(std::shared_ptr<grpc::Channel> channel);

    void sendMessage(const std::string& fieldName,
                     const google::protobuf::Message& message);

    void SendAllMessages(const UniversalMessage& message);

    static bool CopyFieldValue(google::protobuf::Message& dst,
        const google::protobuf::Message& src,
        const google::protobuf::FieldDescriptor* field);

    static bool CopySingularField(
        google::protobuf::Message& dst,
        const google::protobuf::Message& src,
        const google::protobuf::FieldDescriptor* field);

    static bool CopyRepeatedField(
        google::protobuf::Message& dst,
        const google::protobuf::Message& src,
        const google::protobuf::FieldDescriptor* field);

    std::vector<std::pair<std::string, const google::protobuf::FieldDescriptor*>>
    GetSetFields(const UniversalMessage& message);

private:
    std::unique_ptr<UniversalTester::Stub> stub_;
};
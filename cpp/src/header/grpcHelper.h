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
                     const google::protobuf::Message& message,
                     const std::filesystem::path &filePath, 
                     const std::string& timestamp);

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

    void setFieldsWithConfigValues(std::vector<std::string>, UniversalMessage& message);

    void helperSetAllOptionalIndividualFields(UniversalMessage& message, std::string flag, std::string value);

    void helperSetAllIndividualFields(UniversalMessage& message, std::string flag, std::string value);

    void helperSetAllRepeatedFields(UniversalMessage& message, std::string flag, std::string value);

    void helperSetAllMapFields(UniversalMessage& message, std::string flag, std::string value);

    void helperSetNestedFields(UniversalMessage& message, std::string flag, std::string value);

    void helperSetStatusFields(UniversalMessage& message, std::string flag, std::string value);

    std::pair<std::string, std::string> helperGetMapKeyAndValueFromString(std::string value);

    struct ParsedString {
        std::string first;
        std::string innerKey;
        std::string innerValue;
    };

    ParsedString parseString(const std::string& input);

    std::vector<char> extractCharsFromFlag(std::string& input);

    std::string currentTimestamp();

private:
    std::unique_ptr<UniversalTester::Stub> stub_;

    UniversalMessage sendUniversalWithTimestamp(const UniversalMessage& msg,
                                            const std::string& timestamp);
    
    void withTimestamp(grpc::ClientContext& context, const std::string& timestamp);
    
    static const std::unordered_map<std::string, UniversalMessage::Status> statusMap;
};
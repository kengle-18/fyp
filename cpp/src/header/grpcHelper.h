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
    explicit GrpcClient(std::shared_ptr<Channel> channel)
        : stub_(UniversalTester::NewStub(channel)) {}

    void SendMessage(const std::string& fieldName, const google::protobuf::Message& message) {
        UniversalMessage msgToSend;
        const google::protobuf::Descriptor* desc = UniversalMessage::descriptor();
        const google::protobuf::Reflection* refl = msgToSend.GetReflection();

        const google::protobuf::FieldDescriptor* field = desc->FindFieldByName(fieldName);
        if (!field) {
            throw std::invalid_argument("Unknown field: " + fieldName);
        }

        // Copy value from another message with same field set
        const google::protobuf::Reflection* srcRefl = message.GetReflection();
        if (field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_INT32) {
            refl->SetInt32(&msgToSend, field, srcRefl->GetInt32(message, field));
        } else if (field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_BOOL) {
            refl->SetBool(&msgToSend, field, srcRefl->GetBool(message, field));
        } else if (field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_STRING) {
            refl->SetString(&msgToSend, field, srcRefl->GetString(message, field));
        } else {
            std::cerr << "Unhandled field type for " << fieldName << "\n";
            return;
        }

        UniversalMessage response;
        ClientContext context;
        Status status = stub_->SendUniversal(&context, msgToSend, &response);

        if (status.ok()) {
            std::cout << "Sent field " << fieldName << ", got response: "
                      << response.DebugString() << "\n";
        } else {
            std::cerr << "RPC failed: " << status.error_message() << "\n";
        }
    }

    void SendAllMessages(const UniversalMessage& message) {
        auto setFields = GetSetFields(message);
        for (auto& [fieldName, _] : setFields) {
            std::cout << "Sending field " << fieldName << "...\n";
            SendMessage(fieldName, message);
        }
    }

    std::vector<std::pair<std::string, const google::protobuf::FieldDescriptor*>> 
    GetSetFields(const UniversalMessage& message) {
        std::vector<std::pair<std::string, const google::protobuf::FieldDescriptor*>> fields;

        const google::protobuf::Descriptor* desc = message.GetDescriptor();
        const google::protobuf::Reflection* refl = message.GetReflection();

        for (int i = 0; i < desc->field_count(); ++i) {
            const auto* field = desc->field(i);
            if (field->is_repeated()) {
                if (refl->FieldSize(message, field) > 0)
                    fields.emplace_back(field->name(), field);
            } else if (field->is_map()) {
                if (refl->FieldSize(message, field) > 0)
                    fields.emplace_back(field->name(), field);
            } else if (refl->HasField(message, field)) {
                fields.emplace_back(field->name(), field);
            }
        }
        return fields;
    }

private:
    std::unique_ptr<UniversalTester::Stub> stub_;
};
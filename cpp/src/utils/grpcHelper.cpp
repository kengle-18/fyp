#include "../header/grpcHelper.h"

// ===================================
// GreeterServiceImpl (Server-side)
// ===================================
grpc::Status GreeterServiceImpl::SayHello(grpc::ServerContext* context, const HelloRequest* request,
                                          HelloReply* reply) {
    // Create file to test 
    std::ofstream serverFile("/app/cpp/src/server.txt", std::ios::trunc);
    if (serverFile.is_open()) {
        serverFile << R"(Hello from the server side!)";
        serverFile.close();
    }
    reply->set_message("Hello, " + request->name() + "!");
    return grpc::Status::OK;
}

grpc::Status UniversalTesterImpl::SendUniversal(grpc::ServerContext* context, const UniversalMessage* request,
                                          UniversalMessage* reply) {
    const auto* desc = request->GetDescriptor();
    const auto* refl = request->GetReflection();

    auto* replyRefl = reply->GetReflection();

    for (int i = 0; i < desc->field_count(); ++i) {
        const auto* field = desc->field(i);

        std::cout << "[SERVER] Inspecting field: " << field->name()
                    << ", type: " << field->cpp_type_name()
                    << ", repeated: " << field->is_repeated() << "\n";
        
        bool shouldCopy = field->is_repeated() || refl->HasField(*request, field);
        if (!shouldCopy) continue;

        if (!GrpcClient::CopyFieldValue(*reply, *request, field)) {
                std::cerr << "Unhandled field type: " << field->name() << std::endl;
                // Return gRPC UNIMPLEMENTED status
                return grpc::Status(grpc::StatusCode::UNIMPLEMENTED,
                                    "Unhandled field type: " + field->name());
        }
    }
    return grpc::Status::OK;
}



// ===================================
// GreeterClient (Client-side)
// ===================================
GreeterClient::GreeterClient(std::shared_ptr<grpc::Channel> channel)
    : stub_(Greeter::NewStub(channel)) {}

std::string GreeterClient::SayHello(const std::string& name) {
    HelloRequest request;
    request.set_name(name);

    HelloReply reply;
    grpc::ClientContext context;

    grpc::Status status = stub_->SayHello(&context, request, &reply);
    if (status.ok()) {
        std::ofstream serverFile("/app/cpp/src/client.txt", std::ios::trunc);
        if (serverFile.is_open()) {
            serverFile << R"(Hello from the client side!)";
            serverFile.close();
        }
        return reply.message();
    } else {
        return "RPC failed: " + status.error_message();
    }
}

GrpcClient::GrpcClient(std::shared_ptr<grpc::Channel> channel)
    : stub_(UniversalTester::NewStub(channel)) {}

// SendMessage
void GrpcClient::sendMessage(const std::string& fieldName,
                             const google::protobuf::Message& message)
{
    UniversalMessage msgToSend;
    const auto* desc = UniversalMessage::descriptor();
    const auto* field = desc->FindFieldByName(fieldName);

    if (!field)
        throw std::invalid_argument("Unknown field: " + fieldName);

    // Send one field in client message
    if (!CopyFieldValue(msgToSend, message, field)) {
        std::cerr << "Unhandled field type for " << fieldName << "\n";
        return;
    }

    UniversalMessage response;
    ClientContext context;
    Status status = stub_->SendUniversal(&context, msgToSend, &response);

    if (status.ok()) {
        std::cout << "Sent field " << fieldName << "\n";
        std::cout << "got response:\n" << response.DebugString() << std::endl;
    } else {
        std::cerr << "RPC failed: " << status.error_message() << fieldName << "\n";
    }
}

bool GrpcClient::CopyFieldValue(
    google::protobuf::Message& dst,
    const google::protobuf::Message& src,
    const google::protobuf::FieldDescriptor* field)
{

    if (!field) return false;

    if (field->is_repeated()) {
        return CopyRepeatedField(dst, src, field);
    } else {
        return CopySingularField(dst, src, field);
    }
}

bool GrpcClient::CopyRepeatedField(
    google::protobuf::Message& dst,
    const google::protobuf::Message& src,
    const google::protobuf::FieldDescriptor* field)
{
    const auto* srcRefl = src.GetReflection();
    const auto* dstRefl = dst.GetReflection();
    int size = srcRefl->FieldSize(src, field);

    for (int i = 0; i < size; ++i) {
        switch (field->cpp_type()) {
            case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
                dstRefl->AddInt32(&dst, field, srcRefl->GetRepeatedInt32(src, field, i));
                break;
            case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
                dstRefl->AddInt64(&dst, field, srcRefl->GetRepeatedInt64(src, field, i));
                break;
            case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
                dstRefl->AddBool(&dst, field, srcRefl->GetRepeatedBool(src, field, i));
                break;
            case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
                dstRefl->AddString(&dst, field, srcRefl->GetRepeatedString(src, field, i));
                break;
            case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
                dstRefl->AddFloat(&dst, field, srcRefl->GetRepeatedFloat(src, field, i));
                break;
            case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
                dstRefl->AddDouble(&dst, field, srcRefl->GetRepeatedDouble(src, field, i));
                break;
            case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:
                dstRefl->AddEnum(&dst, field, srcRefl->GetRepeatedEnum(src, field, i));
                break;
            case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
                dstRefl->AddMessage(&dst, field)
                       ->CopyFrom(srcRefl->GetRepeatedMessage(src, field, i));
                break;
            default:
                return false;
        }
    }
    return true;
}

bool GrpcClient::CopySingularField(
    google::protobuf::Message& dst,
    const google::protobuf::Message& src,
    const google::protobuf::FieldDescriptor* field)
{
    const auto* srcRefl = src.GetReflection();
    const auto* dstRefl = dst.GetReflection();

    switch (field->cpp_type()) {
        case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
            dstRefl->SetInt32(&dst, field, srcRefl->GetInt32(src, field));
            return true;
        case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
            dstRefl->SetInt64(&dst, field, srcRefl->GetInt64(src, field));
            return true;
        case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
            dstRefl->SetBool(&dst, field, srcRefl->GetBool(src, field));
            return true;
        case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
            dstRefl->SetString(&dst, field, srcRefl->GetString(src, field));
            return true;
        case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
            dstRefl->SetFloat(&dst, field, srcRefl->GetFloat(src, field));
            return true;
        case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
            dstRefl->SetDouble(&dst, field, srcRefl->GetDouble(src, field));
            return true;
        case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:
            dstRefl->SetEnum(&dst, field, srcRefl->GetEnum(src, field));
            return true;
        case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
            dstRefl->MutableMessage(&dst, field)
                   ->CopyFrom(srcRefl->GetMessage(src, field));
            return true;
        default:
            return false;
    }
}

// SendAllMessages
void GrpcClient::SendAllMessages(const UniversalMessage& message)
{
    auto fields = GetSetFields(message);
    for (auto& [name, _] : fields) {
        std::cout << "Sending field " << name << "...\n";
        GrpcClient::sendMessage(name, message);
    }

    std::cout << "Sending message full " << std::endl;

    UniversalMessage reply;
    grpc::ClientContext context;

    grpc::Status status = stub_->SendUniversal(&context, message, &reply);
    if (!status.ok()) {
        std::cerr << "Failed to send message: "
                  << status.error_code() << " - "
                  << status.error_message() << std::endl;
        return;
    }

    std::cout << "Message sent successfully. Server replied with:\n"
              << reply.DebugString() << std::endl;

}

// GetSetFields
std::vector<std::pair<std::string, const google::protobuf::FieldDescriptor*>>
GrpcClient::GetSetFields(const UniversalMessage& message)
{
    std::vector<std::pair<std::string, const google::protobuf::FieldDescriptor*>> out;

    const auto* desc = message.GetDescriptor();
    const auto* refl = message.GetReflection();

    for (int i = 0; i < desc->field_count(); ++i) {
        const auto* field = desc->field(i);

        if (field->is_repeated() || field->is_map()) {
            if (refl->FieldSize(message, field) > 0)
                out.emplace_back(field->name(), field);
        } else if (refl->HasField(message, field)) {
            out.emplace_back(field->name(), field);
        }
    }
    return out;
}

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
void GrpcClient::SendMessage(const std::string& fieldName,
                             const google::protobuf::Message& message)
{
    UniversalMessage msgToSend;
    const auto* desc = UniversalMessage::descriptor();
    const auto* refl = msgToSend.GetReflection();

    const auto* field = desc->FindFieldByName(fieldName);
    if (!field)
        throw std::invalid_argument("Unknown field: " + fieldName);

    const auto* srcRefl = message.GetReflection();

    switch (field->cpp_type()) {
        case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
            refl->SetInt32(&msgToSend, field, srcRefl->GetInt32(message, field));
            break;
        case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
            refl->SetBool(&msgToSend, field, srcRefl->GetBool(message, field));
            break;
        case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
            refl->SetString(&msgToSend, field, srcRefl->GetString(message, field));
            break;
        default:
            std::cerr << "Unhandled field type for " << fieldName << "\n";
            return;
    }

    UniversalMessage response;
    ClientContext context;
    Status status = stub_->SendUniversal(&context, msgToSend, &response);

    if (status.ok()) {
        std::cout << "Sent field " << fieldName << ", got response:\n"
                  << response.DebugString() << "\n";
    } else {
        std::cerr << "RPC failed: " << status.error_message() << "\n";
    }
}

// SendAllMessages
void GrpcClient::SendAllMessages(const UniversalMessage& message)
{
    auto fields = GetSetFields(message);
    for (auto& [name, _] : fields) {
        std::cout << "Sending field " << name << "...\n";
        SendMessage(name, message);
    }
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

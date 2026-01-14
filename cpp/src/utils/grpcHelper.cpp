#include "../header/grpcHelper.h"
#include "../header/io.h"
#include "../header/captureAllHeadersInterceptor.h"
#include <algorithm>
#include <google/protobuf/util/json_util.h>

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

    // header 
    const char* env = std::getenv("OUTPUT_DIR_GENERATED");
    const char* fileEnv1 = std::getenv("OUTPUT_FILE_CPPHEADER_AT_SERVER");
    std::filesystem::path outputFile1 = fileEnv1 ? std::filesystem::path(fileEnv1) : std::filesystem::path("default.txt");
    // std::filesystem::path cppHeaderAtServer = std::filesystem::path(env ? env : ".") / (currentTimeStamp + "_" + outputFile1.filename().string());
    
    const char* fileEnv2 = std::getenv("OUTPUT_FILE_CPP_REQUEST_FROM_SERVER");
    std::filesystem::path outputFile2 = fileEnv2 ? std::filesystem::path(fileEnv2) : std::filesystem::path("default.txt");

    std::cout << "=== ALL INCOMING HEADERS in server ===" << std::endl;
    const std::multimap<grpc::string_ref, grpc::string_ref>& metadata = 
            context->client_metadata();
        
    std::string timestamp = "N/A";
    std::ostringstream headersToWrite;
    
    for (const auto& pair : metadata) {
        std::string key(pair.first.data(), pair.first.size());
        std::string value(pair.second.data(), pair.second.size());
        
        if (key == "timestamp") {
            timestamp = value;
            continue;
        }

        // Save other headers
        headersToWrite << key << " -> " << value << "\n";
    }
    if(!timestamp.empty()){
        std::filesystem::path cppHeaderAtServer = std::filesystem::path(env ? env : ".") / (timestamp + "_" + outputFile1.filename().string());
        FileUtils::appendToFile(cppHeaderAtServer.string(), headersToWrite.str());
    } else {
        std::cout << "Error in printing headers from server" << std::endl;
    }

    std::filesystem::path cppRequestFromServer = std::filesystem::path(env ? env : ".") / (timestamp + "_" + outputFile2.filename().string());
    FileUtils::appendFullMessageWithDefaultsAsPrettyJson(cppRequestFromServer, *request);

    std::cout << "Done in printing header and fields in server" << std::endl;

    // for (int i = 0; i < desc->field_count(); ++i) {
    //     const auto* field = desc->field(i);

    //     std::cout << "[SERVER] Inspecting field: " << field->name()
    //                 << ", type: " << field->cpp_type_name()
    //                 << ", repeated: " << field->is_repeated() << "\n";
                
        // bool shouldCopy = field->is_repeated() || refl->HasField(*request, field);
        // if (!shouldCopy) continue;

        // if (!GrpcClient::CopyFieldValue(*reply, *request, field)) {
        //         std::cerr << "Unhandled field type: " << field->name() << std::endl;
        //         // Return gRPC UNIMPLEMENTED status
        //         return grpc::Status(grpc::StatusCode::UNIMPLEMENTED,
        //                             "Unhandled field type: " + field->name());
        // }
    // }
    reply->CopyFrom(*request);
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
                             const google::protobuf::Message& message,
                             const std::filesystem::path &filePath,
                             const std::string& timestamp)
{
    UniversalMessage msgToSend;
    const auto* desc = UniversalMessage::descriptor();
    const auto* field = desc->FindFieldByName(fieldName);

    // std::cout << "Field name: " << field->name() << std::endl;

    if (!field)
        throw std::invalid_argument("Unknown field: " + fieldName);

    // Send one field in client message
    if (!CopyFieldValue(msgToSend, message, field)) {
        std::cerr << "Unhandled field type for " << fieldName << "\n";
        return;
    }

    UniversalMessage response;
    ClientContext context;

    // try add timestamp in cpp
    withTimestamp(context, timestamp);

    Status status = stub_->SendUniversal(&context, msgToSend, &response);

    if (status.ok()) {
        // std::cout << "ok" << std::endl;
        FileUtils::appendFullMessageWithDefaultsAsPrettyJson(filePath,response);
    } else {
        std::cerr << "RPC failed: " << status.error_message() << fieldName << "\n";
    }
}

UniversalMessage GrpcClient::sendUniversalWithTimestamp(const UniversalMessage& msg,
                                                    const std::string& timestamp) {
    UniversalMessage response;
    grpc::ClientContext context;
    withTimestamp(context, timestamp);
    grpc::Status status = stub_->SendUniversal(&context, msg, &response);

    if (!status.ok()) {
        throw std::runtime_error("RPC failed: " + status.error_message());
    }

    return response;
}

void GrpcClient::withTimestamp(grpc::ClientContext& context, const std::string& timestamp) {
    context.AddMetadata("timestamp", timestamp);
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
    // std::cout << "Size:" << size << std::endl; 

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

    // std::cout << "Indiv:" << field->cpp_type() << " proto_type=" << field->type_name() << std::endl;

    // for optional if set then copy else skip
    // has_presence() can track?, false for non-optional scalar fields in proto3 and repeated fields
    // hasField return false if optional never set, or non-optioanl scaler
    // if (field->has_presence() && !srcRefl->HasField(src, field)) {
    //     return true;
    // }
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
    const char* env = std::getenv("OUTPUT_DIR_GENERATED");
    const char* fileEnv1 = std::getenv("OUTPUT_FILE_CPP_MESSAGE_INITAL");
    const char* fileEnv2 = std::getenv("OUTPUT_FILE_CPP_MESSAGE_INDIVIDUAL_FIELD");
    const char* fileEnv3 = std::getenv("OUTPUT_FILE_CPP_RESPONSE_FROM_SERVER");

    auto currentTimeStamp = currentTimestamp();
    // std::cout << currentTimestamp() << std::endl;

    std::filesystem::path outputFile1 = fileEnv1 ? std::filesystem::path(fileEnv1) : std::filesystem::path("default.txt");
    std::filesystem::path outputFile2 = fileEnv2 ? std::filesystem::path(fileEnv2) : std::filesystem::path("default.txt");
    std::filesystem::path outputFile3 = fileEnv3 ? std::filesystem::path(fileEnv3) : std::filesystem::path("default.txt");

    std::filesystem::path cppMessageInital = std::filesystem::path(env ? env : ".") / (currentTimeStamp + "_" + outputFile1.filename().string());
    std::filesystem::path cppMessageIndiviualField = std::filesystem::path(env ? env : ".") / (currentTimeStamp + "_" + outputFile2.filename().string());
    std::filesystem::path cppResponseFromServer = std::filesystem::path(env ? env : ".") / (currentTimeStamp + "_" + outputFile3.filename().string());

    //  Full inital message to send
    FileUtils::writeFullMessageWithDefaultsAsPrettyJson(cppMessageInital, message);

    // Reponse full message from server
    UniversalMessage reply = sendUniversalWithTimestamp(message, currentTimeStamp);
    // FileUtils::writeToFile(cppResponseFromServer.string(), reply.DebugString());
    FileUtils::writeFullMessageWithDefaultsAsPrettyJson(cppResponseFromServer, reply);

    if (!FileUtils::writeToFile(cppMessageIndiviualField.string(), "=== Sending individual fields ===")){
        std::cout << "Error opening file" << std::endl;
    }
    auto fields = GetSetFields(message);
    for (auto& [name, _] : fields) {
        GrpcClient::sendMessage(name, message, cppMessageIndiviualField, currentTimeStamp);
    }
    FileUtils::appendToFile(cppMessageIndiviualField.string(), "=== End of individual fields ===");

    // Write as a full message
    // std::cout << "Sending message full " << std::endl;

    // UniversalMessage reply;
    // grpc::ClientContext context;

    // grpc::Status status = stub_->SendUniversal(&context, message, &reply);
    // if (!status.ok()) {
    //     std::cerr << "Failed to send message: "
    //               << status.error_code() << " - "
    //               << status.error_message() << std::endl;
    //     return;
    // }

    // std::cout << "Message sent successfully. Server replied with:\n"
    //           << reply.DebugString() << std::endl;
    // FileUtils::writeToFile(cppResponseFromServer.string(), reply.DebugString());

}


std::string GrpcClient::currentTimestamp()
{
    using namespace std::chrono;

    auto now = system_clock::now();
    auto time_t_now = system_clock::to_time_t(now);
    auto ms = duration_cast<milliseconds>(
        now.time_since_epoch()) % 1000;

    std::tm tm_now;
    #if defined(_WIN32)
        localtime_s(&tm_now, &time_t_now);
    #else
        localtime_r(&time_t_now, &tm_now);
    #endif

    std::ostringstream oss;
    oss << std::put_time(&tm_now, "%Y%m%d_%H%M%S")
        << "_" << std::setw(3) << std::setfill('0') << ms.count();

    return oss.str();
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
        // if soemthing is set but default value in scaler is not set explicilty
        } else if (refl->HasField(message, field)) {
            out.emplace_back(field->name(), field);
        }
    }
    return out;
}

void GrpcClient::setFieldsWithConfigValues(std::vector<std::string> configValues, UniversalMessage& message) {
    for (size_t i = 0; i + 1 < configValues.size(); i += 2) {
        std::string& flag = configValues[i];
        const std::string& value = configValues[i + 1];

        // std::cout << "flag: " << flag << ", Value: " << value << "\n";

        std::vector<char> chars = extractCharsFromFlag(flag);
        char prefix = chars[0];
        flag.erase(0, 1);
        // std::cout << "Extracted characters: ";
        // for (char c : chars) {
        //     std::cout << c << " ";
        // }
        // std::cout << std::endl;

        // set singular fields
        switch (prefix) {
            case 'o':
                helperSetAllOptionalIndividualFields(message, flag, value);
                break;
            case 's':
                helperSetAllIndividualFields(message, flag, value);
                break;
            case 'r':
                helperSetAllRepeatedFields(message, flag, value);
                break;
            case 'm':
            // // Only can store one key--> value, write again will cause the values to overwrite
                helperSetAllMapFields(message, flag, value);
                break;
            case 'n':
                helperSetNestedFields(message, flag, value);
                break;
            case 'x':
                helperSetStatusFields(message, flag, value);
                break;
            default:
                std::cerr << "Unknown flag prefix: " << chars[0] << "\n";
                break;
        }

        if (value.empty()){
            std::cout << "Empty value for flag: " << flag << ", skipping to set the value\n";
        }
        // std::cout << message.DebugString() << std::endl;
    }
}

// For optional message
void GrpcClient::helperSetAllOptionalIndividualFields(UniversalMessage& message, std::string flagToDifferiateSingularTypes, std::string value) {
    // std::cout << flagToDifferiateSingularTypes << ": " << value << "\n";
    
    if (flagToDifferiateSingularTypes == "i") {
        // std::cout << "set_set_single_int\n";
        message.set_opt_single_int(std::stoi(value));
    } else if (flagToDifferiateSingularTypes == "bi") {
        // std::cout << "set_big_int\n";
        message.set_opt_big_int(std::stol(value));
    } else if (flagToDifferiateSingularTypes == "s") {
        // std::cout << "set_single_string\n";
        message.set_opt_single_string(value);
    } else if (flagToDifferiateSingularTypes == "b") {
        // std::cout << "set_single_bool\n";
        if (value == "true"){
            message.set_opt_single_bool(true);
        }
        else if (value == "false"){
            message.set_opt_single_bool(false);
        }
    } else if (flagToDifferiateSingularTypes == "d") {
        // std::cout << "set_single_double\n";
        message.set_opt_single_double(std::stod(value));
    } else if (flagToDifferiateSingularTypes == "f") {
        // std::cout << "set_single_float\n";
        message.set_opt_single_float(std::stof(value));
    } else if (flagToDifferiateSingularTypes == "bts") {
        // std::cout << "set_single_bytes\n";
        message.set_opt_single_bytes(value);
    } else {
        std::cerr << "Unknown flag type: " << flagToDifferiateSingularTypes << "\n";
    }
}

// For setting individual fields
void GrpcClient::helperSetAllIndividualFields(UniversalMessage& message, std::string flagToDifferiateSingularTypes, std::string value) {
    // std::cout << flagToDifferiateSingularTypes << ": " << value << "\n";
    
    if (flagToDifferiateSingularTypes == "i") {
        // std::cout << "set_set_single_int\n";
        message.set_default_single_int(std::stoi(value));
    } else if (flagToDifferiateSingularTypes == "bi") {
        // std::cout << "set_big_int\n";
        message.set_default_big_int(std::stol(value));
    } else if (flagToDifferiateSingularTypes == "s") {
        // std::cout << "set_single_string\n";
        message.set_default_single_string(value);
    } else if (flagToDifferiateSingularTypes == "b") {
        // std::cout << "set_single_bool\n";
        if (value == "true"){
            message.set_default_single_bool(true);
        }
        else if (value == "false"){
            message.set_default_single_bool(false);
        }
    } else if (flagToDifferiateSingularTypes == "d") {
        // std::cout << "set_single_double\n";
        message.set_default_single_double(std::stod(value));
    } else if (flagToDifferiateSingularTypes == "f") {
        // std::cout << "set_single_float\n";
        message.set_default_single_float(std::stof(value));
    } else if (flagToDifferiateSingularTypes == "bts") {
        // std::cout << "set_single_bytes\n";
        message.set_default_single_bytes(value);
    } else {
        std::cerr << "Unknown flag type: " << flagToDifferiateSingularTypes << "\n";
    }
}

void GrpcClient::helperSetAllRepeatedFields(UniversalMessage& message, std::string flagToDifferiateRepeatedTypes, std::string value){
    // std::cout << flagToDifferiateRepeatedTypes << ": " << value << std::endl;

    if (flagToDifferiateRepeatedTypes == "i") {
        // std::cout << "add_repeated_int\n";
        message.add_repeated_int(std::stoi(value));
    } else if (flagToDifferiateRepeatedTypes == "bi") {
        // std::cout << "add_repeated_big_int\n";
        message.add_repeated_big_int(std::stol(value));
    } else if (flagToDifferiateRepeatedTypes == "s") {
        // std::cout << "add_repeated_string\n";
        message.add_repeated_string(value);
    } else if (flagToDifferiateRepeatedTypes == "b") {
        // std::cout << "add_repeated_bool\n";
        if (value == "true"){
            message.add_repeated_bool(true);
        }
        else if (value == "false"){
            message.add_repeated_bool(false);
        }
    } else if (flagToDifferiateRepeatedTypes == "d") {
        // std::cout << "add_repeated_double\n";
        message.add_repeated_double(std::stod(value));
    } else if (flagToDifferiateRepeatedTypes == "f") {
        // std::cout << "add_repeated_float\n";
        message.add_repeated_float(std::stof(value));
    } else if (flagToDifferiateRepeatedTypes == "bts") {
        // std::cout << "add_repeated_bytes\n";
        message.add_repeated_bytes(value);
    } else if (flagToDifferiateRepeatedTypes == "nsi") {
        // for repeated nested
        auto keyVal = helperGetMapKeyAndValueFromString(value);
        auto* repeatedNested = message.add_repeated_nested(); 
        if (!keyVal.first.empty()){
            repeatedNested->set_name(keyVal.first); 
        }
        if(!keyVal.second.empty()){
            repeatedNested->set_value(std::stoi(keyVal.second));
        }
    } else if (flagToDifferiateRepeatedTypes == "x") {
        std::transform(value.begin(), value.end(), value.begin(), ::toupper);
        // std::cout<< value << std::endl;

        // first is the map key of caps input, second is the UniversalMessage 
        auto it = statusMap.find(value);
        if (it != statusMap.end()) {
            message.add_repeated_status(it->second);
        } else {
            std::cerr << "Unknown status value: " << value << ", setting to UNKNOWN\n";
        }
    } else {
        std::cerr << "Unknown flag type: " << flagToDifferiateRepeatedTypes << "\n";
    }
}

const std::unordered_map<std::string, UniversalMessage::Status> GrpcClient::statusMap = {
    {"UNKNOWN", UniversalMessage::UNKNOWN},
    {"ACTIVE", UniversalMessage::ACTIVE},
    {"INACTIVE", UniversalMessage::INACTIVE}
};

void GrpcClient::helperSetAllMapFields(UniversalMessage& message, std::string flagToDifferiateMapTypes, std::string value){
    // std::cout << flagToDifferiateMapTypes << ": " << value << std::endl;

    auto keyVal = helperGetMapKeyAndValueFromString(value);

    if (flagToDifferiateMapTypes == "is") {
        // Add key (int) --> value (string)
        // std::cout << "mutable_map_int_string\n";
        (*message.mutable_map_int_string())[std::stoi(keyVal.first)] = keyVal.second;
    } else if (flagToDifferiateMapTypes == "si") {
        // std::cout << "mutable_map_string_int\n";
        auto& mapRef = *message.mutable_map_string_int();
        auto& entry = mapRef[keyVal.first];
        if (!keyVal.second.empty()){
            entry = std::stoi(keyVal.second);
        }else{
            // std::cout << "no value" <<std::endl;
            // std::cout << "Key: " << keyVal.first
            // << ", Value: " << entry<< std::endl;
        }
    }
    else if (flagToDifferiateMapTypes == "in"){
        auto parsed = this->parseString(value);
        // std::cout << parsed.first << ", " << parsed.innerKey << ", " << parsed.innerValue << std::endl;
        // must have key outer
        auto& nestedMap = (*message.mutable_map_int_nested())[std::stoi(parsed.first)];

        if (!parsed.innerKey.empty()){
            nestedMap.set_name(parsed.innerKey);
        }

        if (!parsed.innerValue.empty()){
            nestedMap.set_value(std::stoi(parsed.innerValue));
        }

    } else {
        std::cerr << "Unknown flag type: " << flagToDifferiateMapTypes << "\n";
    }
}

void GrpcClient::helperSetNestedFields(UniversalMessage& message, std::string flag, std::string value){
    // std::cout << flag << ": " << value << std::endl;
    auto keyVal = helperGetMapKeyAndValueFromString(value);
    // std::cout << "first part of value: "<< keyVal.first << ", second part of value: " << keyVal.second << std::endl;
    if(flag == "si"){
        auto* nested = message.mutable_nested();
        if (!keyVal.first.empty()){
            nested->set_name(keyVal.first);
        }
        if (!keyVal.second.empty()){
            nested->set_value(std::stoi(keyVal.second));
        }
    } else {
        std::cerr << "Unknown flag type: " << flag << "\n";
    }
}

 void GrpcClient::helperSetStatusFields(UniversalMessage& message, std::string flag, std::string value){
    // std::cout << flag << ": " << value << std::endl;

    std::transform(value.begin(), value.end(), value.begin(), ::toupper);
    // std::cout<< value << std::endl;

    // first is the map key of caps input, second is the UniversalMessage 
    auto it = statusMap.find(value);
    if (it != statusMap.end()) {
        message.set_status(it->second);
    } else {
        std::cerr << "Unknown status value: " << value << ", not setting enum\n";
    }
    
 }

GrpcClient::ParsedString GrpcClient::parseString(const std::string& input){
    ParsedString result;
    std::stringstream ss(input);
    std::string part;

     // First part: before first comma
    if (std::getline(ss, part, ',')) {
        result.first = part;
    }

    // Second part: innerkey=inner1
    if (std::getline(ss, part, ',')) {
        size_t pos = part.find("=");
        if (pos != std::string::npos) {
            result.innerKey = part.substr(pos + 1);
        }
    }

    // Third part: innervalue=100
    if (std::getline(ss, part, ',')) {
        size_t pos = part.find("=");
        if (pos != std::string::npos) {
            result.innerValue = part.substr(pos + 1);
        }
    }

    return result;
}

// Works for nested and map
std::pair<std::string, std::string> GrpcClient::helperGetMapKeyAndValueFromString(std::string value){
    size_t firstComma = value.find(',');
    // No comma found
    if (firstComma == std::string::npos) {
        return {"", ""};
    }

    std::string key = value.substr(0, firstComma);
    std::string val = value.substr(firstComma + 1);

    return {key, val};
}

std::vector<char> GrpcClient::extractCharsFromFlag(std::string& input){
    std::vector<char> result;

    if (input.rfind("--", 0) == 0) {  // rfind with pos 0 checks if string starts with "--"
        input.erase(0, 2);            // remove the first two characters from index 0 
    }
    for (char c : input) {
        result.push_back(c);
    }

    return result;
}

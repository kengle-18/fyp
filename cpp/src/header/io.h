#pragma once
#include <string>
#include <fstream>
#include <filesystem>

#include <google/protobuf/message.h>
#include <google/protobuf/util/json_util.h>

namespace FileUtils {

    inline bool writeToFile(const std::string& filename, const std::string& content) {
        std::ofstream file(filename);      // Opens for writing (truncates)
        if (!file.is_open()) {
            return false;                  // Failed to open
        }

        if (!content.empty()){
            file << content << '\n';       // Write content
        }

        return true;                        // Success
    }

    inline bool appendToFile(const std::string& filename, const std::string& content) {
        std::ofstream file(filename, std::ios::app); // Open in append mode
        if (!file.is_open()) {
            return false;                  // Failed to open
        }
        file << content << '\n';            // Append content
        return true;                        // Success 
    }

    inline std::string fullMessageWithDefaultsToPrettyJson(const google::protobuf::Message& message
    ){
        google::protobuf::util::JsonPrintOptions options;
        options.always_print_primitive_fields = true; // include defaults
        options.preserve_proto_field_names = true;    // proto names
        options.add_whitespace = true;                // pretty-print

        std::string json;
        google::protobuf::util::MessageToJsonString(message, &json, options);
        return json;
    }

    inline bool writeFullMessageWithDefaultsAsPrettyJson(
        const std::filesystem::path& filename,
        const google::protobuf::Message& message
    ) {
        return writeToFile(filename.string(), fullMessageWithDefaultsToPrettyJson(message));
    }

    inline bool appendFullMessageWithDefaultsAsPrettyJson(
        const std::filesystem::path& filename,
        const google::protobuf::Message& message
    ) {
        return appendToFile(filename.string(), fullMessageWithDefaultsToPrettyJson(message));
    }

}
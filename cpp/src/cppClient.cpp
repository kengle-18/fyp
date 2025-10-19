#include "header/common.h"
#include "header/grpcHelper.h"

namespace fs = std::filesystem;


// ------------------------------
// Config Helper (Scala-like)
// ------------------------------
class Config {
private:
    std::vector<std::string> args_;
    std::map<int, std::string> fileConfig_;
    std::string configDir_;
    std::string configFile_;

    std::string findDefaultConfigFile() {
        if (!fs::exists(configDir_) || !fs::is_directory(configDir_)) {
            throw std::runtime_error("Config directory not found: " + configDir_);
        }

        // Find first non-empty file other than configFile_
        for (const auto& entry : fs::directory_iterator(configDir_)) {
            if (entry.is_regular_file() && entry.file_size() > 0 &&
                entry.path().filename() != configFile_) {
                return entry.path().string();
            }
        }

        // Fallback: try configFile_
        std::string defaultPath = configDir_ + "/" + configFile_;
        if (fs::exists(defaultPath) && fs::file_size(defaultPath) > 0)
            std::cout << "Using default config file: " << defaultPath << std::endl;
            return defaultPath;

        throw std::runtime_error("No valid config file found in " + configDir_);
    }

    void loadFileConfig(const std::string& filePath) {
        std::ifstream file(filePath);
        if (!file) return;
        std::string line;
        int index = 0;
        while (std::getline(file, line)) {
            if (!line.empty())
                fileConfig_[index++] = line;
        }
    }

public:
    Config(int argc, char** argv) {
        const char* dir_env = std::getenv("CONFIG_DIR");
        const char* file_env = std::getenv("CONFIG_FILE");
        configDir_ = dir_env ? dir_env : "/cpp-dest-test";
        configFile_ = file_env ? file_env : "config.txt";

        // Copy CLI args
        for (int i = 1; i < argc; ++i)
            args_.emplace_back(argv[i]);

        // Find and load config file
        try {
            std::string configPath = findDefaultConfigFile();
            loadFileConfig(configPath);
        } catch (const std::exception& e) {
            std::cerr << "⚠️ Config load warning: " << e.what() << std::endl;
        }
    }

    std::string getArg(int index, const std::string& fallback = "Default") const {
        if (index < (int)args_.size() && !args_[index].empty())
            return args_[index];
        auto it = fileConfig_.find(index);
        if (it != fileConfig_.end())
            return it->second;
        return fallback;
    }

    std::vector<std::string> getAll(const std::vector<std::string>& fallbacks = {}) const {
        size_t maxLength = std::max({args_.size(), fileConfig_.size(), fallbacks.size()});
        std::vector<std::string> result;
        for (size_t i = 0; i < maxLength; ++i) {
            std::string fb = (i < fallbacks.size()) ? fallbacks[i] : "Default";
            result.push_back(getArg((int)i, fb));
        }
        return result;
    }
};


int main(int argc, char** argv) {
    // Default values (can be overridden by environment variables)
    const char* host_env = std::getenv("SERVER_HOST");
    const char* port_env = std::getenv("SERVER_PORT");

    std::string host = host_env ? host_env : "localhost";
    std::string port = port_env ? port_env : "50051";
    std::string server_address = host + ":" + port;

    GreeterClient client(grpc::CreateChannel(
        server_address, grpc::InsecureChannelCredentials()));

    Config config(argc, argv);
    std::string name = config.getArg(0, "DefaultName");
    std::string option = config.getArg(1, "DefaultOption");

    std::cout << "📘 Name selected: " << name << std::endl;
    std::cout << "📘 Option selected: " << option << std::endl;

    auto allArgs = config.getAll({"Default", "Default"});
    std::ostringstream merged;
    for (size_t i = 0; i < allArgs.size(); ++i) {
        merged << allArgs[i];
        if (i + 1 < allArgs.size()) merged << ", ";
    }
    std::cout << "📘 All args merged: " << merged.str() << std::endl;

    std::string user("DockerUserTesting");
    std::string reply = client.SayHello(user);

    std::cout << "👋 Client received: " << reply << std::endl;
    return 0;
}

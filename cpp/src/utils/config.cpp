#include "../header/common.h"
#include "../header/config.h"

namespace fs = std::filesystem;

std::string Config::findDefaultConfigFile() {
    if (!fs::exists(configDir_) || !fs::is_directory(configDir_)) {
        throw std::runtime_error("Config directory not found: " + configDir_);
    }

    // Find first non-empty file other than configFile_
    for (const auto& entry : fs::directory_iterator(configDir_)) {
        if (entry.is_regular_file() && entry.file_size() > 0 &&
            entry.path().filename() != configFile_) {
            std::cout << "Using alternate config file: " << entry.path().string() << std::endl;
            return entry.path().string();
        }
    }

    // Fallback: try configFile_
    std::string defaultPath = configDir_ + "/" + configFile_;
    if (fs::exists(defaultPath) && fs::file_size(defaultPath) > 0) {
        std::cout << "Using default config file: " << defaultPath << std::endl;
        return defaultPath;
    }

    throw std::runtime_error("No valid config file found in " + configDir_);
}

void Config::loadFileConfig(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file) return;

    std::string line;
    int index = 0;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            fileConfig_[index++] = line;
        }
    }
}

Config::Config(int argc, char** argv) {
    const char* dir_env = std::getenv("CONFIG_DIR");
    const char* file_env = std::getenv("CONFIG_FILE");
    configDir_ = dir_env ? dir_env : "/cpp-dest-test";
    configFile_ = file_env ? file_env : "config.txt";

    // Copy CLI args
    for (int i = 1; i < argc; ++i) {
        args_.emplace_back(argv[i]);
    }

    // Find and load config file
    try {
        std::string configPath = findDefaultConfigFile();
        loadFileConfig(configPath);
    } catch (const std::exception& e) {
        std::cerr << "⚠️ Config load warning: " << e.what() << std::endl;
    }
}

std::string Config::getArg(int index, const std::string& fallback) const {
    if (index < static_cast<int>(args_.size()) && !args_[index].empty())
        return args_[index];

    auto it = fileConfig_.find(index);
    if (it != fileConfig_.end())
        return it->second;

    return fallback;
}

std::vector<std::string> Config::getAll(const std::vector<std::string>& fallbacks) const {
    size_t maxLength = std::max({args_.size(), fileConfig_.size(), fallbacks.size()});
    std::vector<std::string> result;

    for (size_t i = 0; i < maxLength; ++i) {
        std::string fb = (i < fallbacks.size()) ? fallbacks[i] : "Default";
        result.push_back(getArg(static_cast<int>(i), fb));
    }

    return result;
}

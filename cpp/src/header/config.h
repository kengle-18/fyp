#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>
#include <map>

class Config {
private:
    std::vector<std::string> args_;
    std::map<int, std::string> fileConfig_;
    std::string configDir_;
    std::string configFile_;

    std::string findDefaultConfigFile();
    void loadFileConfig(const std::string& filePath);

public:
    Config(int argc, char** argv);
    std::vector<std::vector<std::string>> getAllArgsInLine() const;
};

#endif // CONFIG_H

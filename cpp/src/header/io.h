#pragma once
#include <string>
#include <fstream>

namespace FileUtils {

    inline bool writeToFile(const std::string& filename, const std::string& content) {
        std::ofstream file(filename);      // Opens for writing (truncates)
        if (!file.is_open()) {
            return false;                  // Failed to open
        }
        file << content << '\n';                    // Write content

        return true;                        // Success
    }

    inline bool appendToFile(const std::string& filename, const std::string& content) {
        std::ofstream file(filename, std::ios::app); // Open in append mode
        if (!file.is_open()) {
            return false;                  // Failed to open
        }
        file << content << '\n';                    // Append content
        return true;                        // Success 
    }

}
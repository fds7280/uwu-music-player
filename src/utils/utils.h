#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

namespace Utils {
    // Execute command and return output
    std::string exec(const char* cmd);
    
    // File system utilities
    std::string getHomeDirectory();
    std::string getCacheDirectory();
    void ensureDirectoryExists(const std::string& path);
}

#endif // UTILS_H

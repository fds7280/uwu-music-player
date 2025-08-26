#include "utils/utils.h" 
#include <array>
#include <memory>
#include <cstdio>
#include <filesystem>
#include <cstdlib>

namespace fs = std::filesystem;

namespace Utils {
    std::string exec(const char* cmd) {
        std::array<char, 128> buffer;
        std::string result;
        std::unique_ptr<FILE, int(*)(FILE*)> pipe(popen(cmd, "r"), pclose); 
        if (!pipe) {
            throw std::runtime_error("popen() failed!");
        }
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        return result;
    }
    
    std::string getHomeDirectory() {
        return std::string(getenv("HOME"));
    }
    
    std::string getCacheDirectory() {
        return getHomeDirectory() + "/.tui_player_cache";
    }
    
    void ensureDirectoryExists(const std::string& path) {
        fs::create_directories(path);
    }
}

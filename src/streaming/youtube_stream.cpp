#include "streaming/youtube_stream.h"
#include "../utils/utils.h"
#include "../audio/audio_player.h"
#include "../art/ascii_art.h"
#include <sstream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <ncurses.h>
#include <sys/stat.h>
#include <unistd.h>

namespace fs = std::filesystem;

namespace Streaming {
    void cleanupCache(const std::string& cache_dir, const std::string& current_video_id) {
        try {
            for (const auto& entry : fs::directory_iterator(cache_dir)) {
                if (entry.is_regular_file()) {
                    std::string filename = entry.path().filename().string();
                    if (filename != (current_video_id + ".mp3")) {
                        fs::remove(entry.path());
                    }
                }
            }
        } catch (const fs::filesystem_error&) {
            // ignore
        }
    }

    std::vector<SearchResult> searchYouTube(const std::string& query) {
        std::vector<SearchResult> results;
        if (query.empty()) return results;
        std::string command = "yt-dlp \"ytsearch5:" + query + "\" --flat-playlist -j --no-warnings 2>/dev/null";
        mvprintw(LINES - 1, 0, "Searching...");
        refresh();
        std::string output = Utils::exec(command.c_str());
        move(LINES - 1, 0);
        clrtoeol();
        refresh();
        std::stringstream ss(output);
        std::string line;
        while (std::getline(ss, line)) {
            SearchResult result;
            size_t id_pos = line.find("\"id\": \"");
            if (id_pos != std::string::npos) {
                id_pos += 7;
                size_t id_end = line.find("\"", id_pos);
                if (id_end != std::string::npos) result.id = line.substr(id_pos, id_end - id_pos);
            }
            size_t title_pos = line.find("\"title\": \"");
            if (title_pos != std::string::npos) {
                title_pos += 10;
                size_t title_end = line.find("\"", title_pos);
                if (title_end != std::string::npos) result.title = line.substr(title_pos, title_end - title_pos);
            }
            if (!result.id.empty() && !result.title.empty()) results.push_back(result);
        }
        return results;
    }
    
    bool isCached(const std::string& video_id, const std::string& cache_dir) {
        std::string cached_file = cache_dir + "/" + video_id + ".mp3";
        return fs::exists(cached_file);
    }
    
    std::string getCachedFilePath(const std::string& video_id, const std::string& cache_dir) {
        return cache_dir + "/" + video_id + ".mp3";
    }
}

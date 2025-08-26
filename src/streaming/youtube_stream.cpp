#include "streaming/youtube_stream.h" 
#include "../utils/utils.h"
#include "../audio/audio_player.h"
#include <sstream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <ncurses.h>
#include <sys/stat.h>
#include <unistd.h>

namespace fs = std::filesystem;

namespace Streaming {
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
                if (id_end != std::string::npos) {
                    result.id = line.substr(id_pos, id_end - id_pos);
                }
            }
            
            size_t title_pos = line.find("\"title\": \"");
            if (title_pos != std::string::npos) {
                title_pos += 10;
                size_t title_end = line.find("\"", title_pos);
                if (title_end != std::string::npos) {
                    result.title = line.substr(title_pos, title_end - title_pos);
                }
            }
            
            if (!result.id.empty() && !result.title.empty()) {
                results.push_back(result);
            }
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
    
    void progressiveStreamYouTube(const std::string& video_id, 
                                  const std::string& title, 
                                  const std::string& cache_dir) {
        clear();
        int max_y, max_x;
        
        if (Audio::is_playing) {
            Audio::StopAudio();
        }
        
        system("pkill -f yt-dlp");
        system("pkill -f ffmpeg");
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        getmaxyx(stdscr, max_y, max_x);
        
        std::string final_file = cache_dir + "/" + video_id + ".mp3";
        std::string fifo_path = "/tmp/moz_stream_" + video_id + ".fifo";
        
        unlink(fifo_path.c_str());
        
        if (mkfifo(fifo_path.c_str(), 0666) != 0) {
            mvprintw(max_y/2, (max_x-30)/2, "Failed to create pipe!");
            refresh();
            getch();
            return;
        }
        
        WINDOW* status_win = newwin(6, 60, max_y/2 - 3, max_x/2 - 30);
        box(status_win, 0, 0);
        mvwprintw(status_win, 1, 2, "Streaming: %.50s", title.c_str());
        mvwprintw(status_win, 2, 2, "Starting stream...");
        mvwprintw(status_win, 3, 2, "Buffering...");
        wrefresh(status_win);
        
        std::string dl_command = "yt-dlp -f 'bestaudio[ext=m4a]/bestaudio/best' -o - \"https://youtube.com/watch?v=" + 
                                video_id + "\" 2>/dev/null | ffmpeg -i pipe:0 -acodec libmp3lame -ab 192k -f mp3 - 2>/dev/null | " +
                                "tee \"" + final_file + "\" > \"" + fifo_path + "\" &";
        
        int result = system(dl_command.c_str());
        if (result != 0) {
            mvwprintw(status_win, 4, 2, "Failed to start stream!");
            wrefresh(status_win);
            getch();
            delwin(status_win);
            unlink(fifo_path.c_str());
            return;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        mvwprintw(status_win, 4, 2, "Starting playback in 2 seconds...");
        wrefresh(status_win);
        
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        delwin(status_win);
        
        Audio::is_playing = true;
        Audio::is_paused = false;
        
        std::thread play_thread([fifo_path]() {
            Audio::PlayAudio(fifo_path);
        });
        
        clear();
        mvprintw(0, 0, "Now Streaming: %s", title.c_str());
        mvprintw(2, 0, "Press 'q' to stop, SPACE to pause/resume.");
        mvprintw(3, 0, "Streaming in real-time...");
        nodelay(stdscr, TRUE);
        
        while(Audio::is_playing) {
            int ch = getch();
            if (ch == 'q') {
                Audio::is_playing = false;
                system("pkill -f yt-dlp");
                system("pkill -f ffmpeg");
            } else if (ch == ' ') {
                Audio::is_paused = !Audio::is_paused;
                mvprintw(4, 0, Audio::is_paused ? "PAUSED " : "PLAYING");
                clrtoeol();
            }
            
            if (fs::exists(final_file)) {
                size_t size = fs::file_size(final_file);
                mvprintw(5, 0, "Cached: %zu KB", size / 1024);
                clrtoeol();
            }
            
            int current_seconds = Audio::GetCurrentTimeSeconds();
            mvprintw(6, 0, "Time: %02d:%02d", current_seconds / 60, current_seconds % 60);
            clrtoeol();
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        nodelay(stdscr, FALSE);
        
        if (play_thread.joinable()) {
            play_thread.join();
        }
        
        unlink(fifo_path.c_str());
    }
}

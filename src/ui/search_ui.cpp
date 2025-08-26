#include "ui/search_ui.h"
#include "../utils/utils.h"
#include "../audio/audio_player.h"
#include <ncurses.h>
#include <filesystem>
#include <thread>
#include <chrono>

namespace fs = std::filesystem;

namespace UI {
    std::string getSearchQuery(int max_y, int max_x) {
        int height = 3;
        int width = max_x / 2;
        int start_y = (max_y - height) / 2;
        int start_x = (max_x - width) / 2;
        
        WINDOW* search_win = newwin(height, width, start_y, start_x);
        box(search_win, 0, 0);
        mvwprintw(search_win, 1, 1, "Search YouTube: ");
        wrefresh(search_win);
        
        echo();
        char str[200];
        wgetstr(search_win, str);
        noecho();
        
        delwin(search_win);
        return std::string(str);
    }
    
    Streaming::SearchResult selectFromResults(const std::vector<Streaming::SearchResult>& results) {
        if (results.empty()) {
            mvprintw(0, 0, "No results found. Press any key to search again.");
            refresh();
            getch();
            return {};
        }

        int highlight = 0;
        int choice = -1;

        while (true) {
            clear();
            mvprintw(0, 0, "YouTube Search Results (Select with Enter, Esc to cancel):");
            for (size_t i = 0; i < results.size(); ++i) {
                if (i == highlight) attron(A_REVERSE);
                mvprintw(i + 2, 1, "%s", results[i].title.c_str());
                if (i == highlight) attroff(A_REVERSE);
            }
            refresh();

            int ch = getch();
            switch(ch) {
                case KEY_UP:
                    highlight = (highlight > 0) ? highlight - 1 : results.size() - 1;
                    break;
                case KEY_DOWN:
                    highlight = (highlight < results.size() - 1) ? highlight + 1 : 0;
                    break;
                case 10: // Enter
                    choice = highlight;
                    goto end_loop;
                case 27: // Escape
                    return {};
            }
        }

    end_loop:
        return results[choice];
    }
    
    void runOnlineMode() {
        const std::string cache_dir = Utils::getCacheDirectory();
        Utils::ensureDirectoryExists(cache_dir);

        while(true) {
            clear();
            int max_y, max_x;
            getmaxyx(stdscr, max_y, max_x);

            std::string query = getSearchQuery(max_y, max_x);
            if (query.empty()) break;

            auto results = Streaming::searchYouTube(query);
            Streaming::SearchResult selection = selectFromResults(results);

            if (selection.id.empty()) continue;

            std::string cached_file_path = Streaming::getCachedFilePath(selection.id, cache_dir);
            
            if (fs::exists(cached_file_path)) {
                // Stop any existing playback
                if (Audio::is_playing) {
                    Audio::StopAudio();
                }
                system("pkill -f yt-dlp");
                system("pkill -f ffmpeg");
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                
                // Play cached file
                Audio::is_playing = true;
                Audio::is_paused = false;
                Audio::PlayAudio(cached_file_path);
                
                clear();
                mvprintw(0, 0, "Now Playing (Cached): %s", selection.title.c_str());
                mvprintw(2, 0, "Press 'q' to stop and search again, SPACE to pause/resume.");
                nodelay(stdscr, TRUE);
                
                while(Audio::is_playing) {
                    int ch = getch();
                    if (ch == 'q') {
                        Audio::is_playing = false;
                    } else if (ch == ' ') {
                        Audio::is_paused = !Audio::is_paused;
                        mvprintw(3, 0, Audio::is_paused ? "PAUSED " : "PLAYING");
                        clrtoeol();
                    }
                    
                    if (Audio::GetTotalTimeSeconds() > 0) {
                        int current_seconds = Audio::GetCurrentTimeSeconds();
                        int total_seconds = Audio::GetTotalTimeSeconds();
                        mvprintw(4, 0, "Time: %02d:%02d / %02d:%02d", 
                                 current_seconds / 60, current_seconds % 60,
                                 total_seconds / 60, total_seconds % 60);
                        clrtoeol();
                    }
                    
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
                nodelay(stdscr, FALSE);
            } else {
                // Stream progressively
                Streaming::progressiveStreamYouTube(selection.id, selection.title, cache_dir);
            }
        }
    }
}

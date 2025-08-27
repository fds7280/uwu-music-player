#include "ui/search_ui.h"
#include "../utils/utils.h"
#include "../audio/audio_player.h"
#include "../streaming/youtube_stream.h"
#include "../art/ascii_art.h"
#include <ncurses.h>
#include <filesystem>
#include <thread>
#include <chrono>
#include <unistd.h>      // for unlink
#include <sys/stat.h>    // for mkfifo
#include <sys/types.h>   // for mkfifo

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
                case KEY_UP: highlight = (highlight > 0) ? highlight - 1 : results.size() - 1; break;
                case KEY_DOWN: highlight = (highlight < results.size() - 1) ? highlight + 1 : 0; break;
                case 10: choice = highlight; goto end_loop;
                case 27: return {};
            }
        }
    end_loop:
        return results[choice];
    }
    
    // This is the function that was missing!
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

            // Stop any previous playback and clean the cache
            if (Audio::is_playing) Audio::StopAudio();
            Streaming::cleanupCache(cache_dir, selection.id);
            [[maybe_unused]] int ret1 = system("pkill -f yt-dlp");
            [[maybe_unused]] int ret2 = system("pkill -f ffmpeg");
            std::this_thread::sleep_for(std::chrono::milliseconds(200));

            // Start streaming to a FIFO and caching the file
            std::string final_file = Streaming::getCachedFilePath(selection.id, cache_dir);
            std::string fifo_path = "/tmp/uwu_stream_" + selection.id + ".fifo";
            unlink(fifo_path.c_str());
            if (mkfifo(fifo_path.c_str(), 0666) != 0) continue;

            std::string dl_command = "yt-dlp -f 'bestaudio[ext=m4a]/bestaudio/best' -o - "
                                     "\"https://youtube.com/watch?v=" + selection.id + "\" 2>/dev/null | "
                                     "ffmpeg -i pipe:0 -acodec libmp3lame -ab 192k -f mp3 - 2>/dev/null | "
                                     "tee \"" + final_file + "\" > \"" + fifo_path + "\" &";
            [[maybe_unused]] int ret3 = system(dl_command.c_str());
            std::this_thread::sleep_for(std::chrono::seconds(2));

            // Play from FIFO
            Audio::is_playing = true;
            Audio::is_paused = false;
            std::thread play_thread([fifo_path]() { Audio::PlayAudio(fifo_path); });

            // Display UI while playing
            nodelay(stdscr, TRUE);
            while(Audio::is_playing) {
                clear();
                getmaxyx(stdscr, max_y, max_x);
                mvprintw(0, 0, "Now Streaming: %s", selection.title.c_str());
                mvprintw(2, 0, "Press 'q' to stop, SPACE to pause/resume.");
                
                WINDOW *info_win = newwin(max_y, max_x / 2, 0, max_x / 2);
                box(info_win, 0, 0);
                mvwprintw(info_win, 1, 1, "YouTube Stream");
                mvwprintw(info_win, 2, 1, "Title: %.30s", selection.title.c_str());
                
                std::vector<std::string> asciiArt = AsciiArt::extractAlbumArtASCII(final_file);
                int art_start_y = 4;
                for (size_t i = 0; i < asciiArt.size(); ++i) {
                    mvwprintw(info_win, art_start_y + i, 1, "%s", asciiArt[i].c_str());
                }
                
                int current_seconds = Audio::GetCurrentTimeSeconds();
                mvwprintw(info_win, art_start_y + AsciiArt::THUMBNAIL_HEIGHT + 1, 1, "Time: %02d:%02d", current_seconds / 60, current_seconds % 60);
                if (fs::exists(final_file)) mvwprintw(info_win, max_y - 3, 1, "Cached: %zu KB", fs::file_size(final_file) / 1024);
                mvwprintw(info_win, max_y - 2, 1, Audio::is_paused ? "PAUSED" : "PLAYING");
                
                refresh();
                wrefresh(info_win);
                delwin(info_win);
                
                int ch = getch();
                if (ch == 'q') Audio::is_playing = false;
                else if (ch == ' ') Audio::is_paused = !Audio::is_paused;
                
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            nodelay(stdscr, FALSE);
            [[maybe_unused]] int ret4 = system("pkill -f yt-dlp");
            [[maybe_unused]] int ret5 = system("pkill -f ffmpeg");
            if (play_thread.joinable()) play_thread.join();
            unlink(fifo_path.c_str());
        }
    }
}

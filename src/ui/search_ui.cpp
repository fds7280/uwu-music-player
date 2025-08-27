#include "ui/search_ui.h"
#include "../utils/utils.h"
#include "../audio/audio_player.h"
#include "../streaming/youtube_stream.h"
#include "../art/ascii_art.h"
#include <ncurses.h>
#include <filesystem>
#include <thread>
#include <chrono>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <algorithm>
#include <ctime>

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
    
    void playYouTubeVideo(const Streaming::SearchResult& video) {
        const std::string cache_dir = Utils::getCacheDirectory();
        
        // Stop any previous playback and clean the cache
        if (Audio::is_playing) Audio::StopAudio();
        Streaming::cleanupCache(cache_dir, video.id);
        [[maybe_unused]] int ret1 = system("pkill -f yt-dlp");
        [[maybe_unused]] int ret2 = system("pkill -f ffmpeg");
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        // Start streaming to a FIFO and caching the file
        std::string final_file = Streaming::getCachedFilePath(video.id, cache_dir);
        std::string fifo_path = "/tmp/uwu_stream_" + video.id + ".fifo";
        unlink(fifo_path.c_str());
        if (mkfifo(fifo_path.c_str(), 0666) != 0) return;

        std::string dl_command = "yt-dlp -f 'bestaudio[ext=m4a]/bestaudio/best' -o - "
                                 "\"https://youtube.com/watch?v=" + video.id + "\" 2>/dev/null | "
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
            int max_y, max_x;
            getmaxyx(stdscr, max_y, max_x);
            mvprintw(0, 0, "Now Streaming: %s", video.title.c_str());
            mvprintw(2, 0, "Press 'q' to stop, SPACE to pause/resume.");
            
            WINDOW *info_win = newwin(max_y, max_x / 2, 0, max_x / 2);
            box(info_win, 0, 0);
            mvwprintw(info_win, 1, 1, "YouTube Stream");
            mvwprintw(info_win, 2, 1, "Title: %.30s", video.title.c_str());
            
            std::vector<std::string> asciiArt = AsciiArt::getYouTubeThumbnailASCII(video.id);
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

            playYouTubeVideo(selection);
        }
    }
    
    void runPlaylistMode() {
        while (true) {
            clear();
            int max_y, max_x;
            getmaxyx(stdscr, max_y, max_x);
            
            mvprintw(0, 0, "=== PLAYLIST MODE ===");
            mvprintw(2, 2, "1. Import YouTube/YouTube Music Playlist");
            mvprintw(3, 2, "2. View Saved Playlists");
            mvprintw(4, 2, "3. Back to Main Menu");
            mvprintw(6, 0, "Select option: ");
            
            int choice = getch();
            
            if (choice == '1') {
                clear();
                mvprintw(0, 0, "Enter YouTube/YouTube Music playlist URL:");
                mvprintw(1, 0, "(Example: https://www.youtube.com/playlist?list=...)");
                mvprintw(3, 0, "URL: ");
                
                echo();
                char url[500];
                getstr(url);
                noecho();
                
                mvprintw(5, 0, "Extracting playlist... This may take a moment...");
                refresh();
                
                Playlist::PlaylistInfo playlist = Playlist::extractYouTubePlaylist(url);
                
                if (!playlist.songs.empty()) {
                    // Generate filename with timestamp
                    std::string safe_name = playlist.name;
                    std::replace(safe_name.begin(), safe_name.end(), ' ', '_');
                    std::replace(safe_name.begin(), safe_name.end(), '/', '-');
                    std::string filename = safe_name + "_" + std::to_string(time(nullptr)) + ".playlist";
                    
                    Playlist::savePlaylist(playlist, filename);
                    
                    mvprintw(7, 0, "Successfully imported %zu songs!", playlist.songs.size());
                    mvprintw(8, 0, "Press any key to view playlist...");
                    getch();
                    
                    viewPlaylist(playlist);
                } else {
                    mvprintw(7, 0, "Failed to extract playlist. Make sure the URL is valid.");
                    mvprintw(8, 0, "Press any key to continue...");
                    getch();
                }
            }
            else if (choice == '2') {
                viewSavedPlaylists();
            }
            else if (choice == '3') {
                break;
            }
        }
    }
    
    void viewSavedPlaylists() {
        auto playlists = Playlist::listSavedPlaylists();
        
        if (playlists.empty()) {
            clear();
            mvprintw(0, 0, "No saved playlists found.");
            mvprintw(2, 0, "Press any key to go back...");
            getch();
            return;
        }
        
        int highlight = 0;
        
        while (true) {
            clear();
            mvprintw(0, 0, "=== SAVED PLAYLISTS ===");
            mvprintw(1, 0, "Select playlist (Enter to open, 'd' to delete, 'q' to go back):");
            
             for (size_t i = 0; i < playlists.size(); i++) {
    if (i == highlight) attron(A_REVERSE);
    
    // Remove .playlist extension for display
    std::string display_name = playlists[i];
    if (display_name.length() >= 9 && display_name.substr(display_name.length() - 9) == ".playlist") {
        display_name = display_name.substr(0, display_name.length() - 9);
    }
    
      mvprintw(i + 3, 2, "%s", display_name.c_str());
    
      if (i == highlight) attroff(A_REVERSE);
        }           
            refresh();
            
            int ch = getch();
            switch(ch) {
                case KEY_UP:
                    highlight = (highlight > 0) ? highlight - 1 : playlists.size() - 1;
                    break;
                case KEY_DOWN:
                    highlight = (highlight < playlists.size() - 1) ? highlight + 1 : 0;
                    break;
                case 10: // Enter
                    {
                        Playlist::PlaylistInfo playlist = Playlist::loadPlaylist(playlists[highlight]);
                        viewPlaylist(playlist);
                    }
                    break;
                case 'd': // Delete
                    {
                        mvprintw(playlists.size() + 4, 0, "Delete this playlist? (y/n)");
                        int confirm = getch();
                        if (confirm == 'y' || confirm == 'Y') {
                            std::string playlist_path = Utils::getCacheDirectory() + "/playlists/" + playlists[highlight];
                            std::filesystem::remove(playlist_path);
                            playlists = Playlist::listSavedPlaylists();
                            if (playlists.empty()) return;
                            if (highlight >= playlists.size()) highlight = playlists.size() - 1;
                        }
                    }
                    break;
                case 'q':
                    return;
            }
        }
    }
    
    void viewPlaylist(const Playlist::PlaylistInfo& playlist) {
        int highlight = 0;
        int scroll_offset = 0;
        
        while (true) {
            clear();
            int max_y, max_x;
            getmaxyx(stdscr, max_y, max_x);
            
            mvprintw(0, 0, "Playlist: %s (%zu songs)", playlist.name.c_str(), playlist.songs.size());
            mvprintw(1, 0, "Enter: Play | Space: Play All | q: Back");
            
            // Calculate display area
            int header_lines = 3;
            int display_lines = max_y - header_lines - 1;
            
            // Adjust scroll offset
            if (highlight < scroll_offset) {
                scroll_offset = highlight;
            } else if (highlight >= scroll_offset + display_lines) {
                scroll_offset = highlight - display_lines + 1;
            }
            
            // Display songs
            for (int i = 0; i < display_lines && scroll_offset + i < playlist.songs.size(); i++) {
                int song_idx = scroll_offset + i;
                const auto& song = playlist.songs[song_idx];
                
                if (song_idx == highlight) attron(A_REVERSE);
                
                               // Format: "  1. Title - Artist [duration]"
                std::string display = std::to_string(song_idx + 1) + ". ";
                display += song.title.substr(0, 40);
                if (song.title.length() > 40) display += "...";
                display += " - " + song.artist.substr(0, 20);
                display += " [" + song.duration + "]";
                
                mvprintw(header_lines + i, 2, "%-*s", max_x - 4, display.c_str());
                
                if (song_idx == highlight) attroff(A_REVERSE);
            }
            
            // Show scroll indicator
            if (playlist.songs.size() > display_lines) {
                mvprintw(max_y - 1, 0, "Song %d/%zu (↑↓ to scroll)", highlight + 1, playlist.songs.size());
            }
            
            refresh();
            
            int ch = getch();
            switch(ch) {
                case KEY_UP:
                    if (highlight > 0) highlight--;
                    break;
                case KEY_DOWN:
                    if (highlight < playlist.songs.size() - 1) highlight++;
                    break;
                case 10: // Enter - play single song
                    {
                        const auto& song = playlist.songs[highlight];
                        Streaming::SearchResult result;
                        result.id = song.id;
                        result.title = song.title + " - " + song.artist;
                        
                        playYouTubeVideo(result);
                    }
                    break;
                case ' ': // Space - play entire playlist
                    playEntirePlaylist(playlist, highlight);
                    break;
                case 'q':
                    return;
            }
        }
    }
    
    void playEntirePlaylist(const Playlist::PlaylistInfo& playlist, int start_index) {
        for (size_t i = start_index; i < playlist.songs.size(); i++) {
            const auto& song = playlist.songs[i];
            
            // Show what's playing
            clear();
            mvprintw(0, 0, "Playing playlist: %s", playlist.name.c_str());
            mvprintw(1, 0, "Track %zu/%zu", i + 1, playlist.songs.size());
            mvprintw(3, 0, "Now playing: %s - %s", song.title.c_str(), song.artist.c_str());
            mvprintw(5, 0, "Press 'n' for next, 'p' for previous, 'q' to stop playlist");
            refresh();
            
            // Prepare the video info
            Streaming::SearchResult result;
            result.id = song.id;
            result.title = song.title + " - " + song.artist;
            
            // Start playback in a separate thread
            std::thread playback_thread([result]() {
                playYouTubeVideo(result);
            });
            
            // Wait for user input or song to finish
            nodelay(stdscr, TRUE);
            bool skip = false;
            bool go_back = false;
            
            while (Audio::is_playing && !skip && !go_back) {
                int ch = getch();
                switch(ch) {
                    case 'n': // Next
                        skip = true;
                        Audio::is_playing = false;
                        break;
                    case 'p': // Previous
                        if (i > 0) {
                            go_back = true;
                            Audio::is_playing = false;
                        }
                        break;
                    case 'q': // Quit playlist
                        Audio::is_playing = false;
                        nodelay(stdscr, FALSE);
                        if (playback_thread.joinable()) playback_thread.join();
                        return;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            
            nodelay(stdscr, FALSE);
            
            // Stop current playback
            Audio::is_playing = false;
            [[maybe_unused]] int ret1 = system("pkill -f yt-dlp");
            [[maybe_unused]] int ret2 = system("pkill -f ffmpeg");
            
            if (playback_thread.joinable()) playback_thread.join();
            
            // Handle navigation
            if (go_back && i > 0) {
                i -= 2; // Will be incremented by loop, so we go back 2 to play previous
            }
        }
        
        // Playlist finished
        clear();
        mvprintw(0, 0, "Playlist finished!");
        mvprintw(2, 0, "Press any key to continue...");
        getch();
    }
}

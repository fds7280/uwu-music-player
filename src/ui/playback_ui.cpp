#include "ui/playback_ui.h"
#include "../audio/audio_player.h"
#include "../metadata/metadata_reader.h"
#include "../art/ascii_art.h"
#include <ncurses.h>
#include <filesystem>
#include <vector>
#include <thread>
#include <chrono>

namespace fs = std::filesystem;

namespace UI {
    void runPlaybackTUI(const std::string& music_directory) {
        std::vector<fs::path> files;
        
        if (!fs::exists(music_directory) || !fs::is_directory(music_directory)) return;
        
        for (const auto& entry : fs::directory_iterator(music_directory)) {
            if (entry.path().extension() == ".mp3") {
                files.push_back(entry.path());
            }
        }

        int selected_item = 0;
        nodelay(stdscr, TRUE);

        while (true) {
            int ch = getch();
            if (ch == 27) break; // Escape to exit

            clear();
            int max_y, max_x;
            getmaxyx(stdscr, max_y, max_x);

            // Draw the song list on the left
            mvprintw(0, 0, "Music in: %s", music_directory.c_str());
            for (size_t i = 0; i < files.size(); ++i) {
                if (i == selected_item) {
                    attron(A_REVERSE);
                    mvprintw(i + 2, 0, "%s", files[i].filename().string().c_str());
                    attroff(A_REVERSE);
                } else {
                    mvprintw(i + 2, 0, "%s", files[i].filename().c_str());
                }
            }
            
            // Draw the playback status on the right
            WINDOW *info_win = newwin(max_y, max_x / 2, 0, max_x / 2);
            box(info_win, 0, 0);

            mvwprintw(info_win, 1, 1, "Now Playing:");
            if (Audio::is_playing && selected_item != -1) {
                Metadata::TrackInfo info = Metadata::readMetadata(files[selected_item].string());
                mvwprintw(info_win, 2, 1, "Title: %s", info.title.c_str());
                mvwprintw(info_win, 3, 1, "Artist: %s", info.artist.c_str());
                mvwprintw(info_win, 4, 1, "Album: %s", info.album.c_str());

                // Draw ASCII art thumbnail
                std::vector<std::string> asciiArt = AsciiArt::extractAlbumArtASCII(files[selected_item].string());
                int art_start_y = 6;
                for (size_t i = 0; i < asciiArt.size() && (art_start_y + i) < (max_y - 8); ++i) {
                    mvwprintw(info_win, art_start_y + i, 1, "%s", asciiArt[i].c_str());
                }

                // Progress bar
                int progress_y = art_start_y + AsciiArt::THUMBNAIL_HEIGHT + 1;
                if (Audio::GetTotalTimeSeconds() > 0 && progress_y < max_y - 4) {
                    float progress = Audio::GetProgress();
                    int bar_width = std::min(AsciiArt::THUMBNAIL_WIDTH, max_x / 2 - 4);
                    int progress_bar_fill = static_cast<int>(bar_width * progress);
                    
                    mvwprintw(info_win, progress_y, 1, "[");
                    for (int i = 0; i < bar_width; ++i) {
                        waddch(info_win, i < progress_bar_fill ? '#' : '-');
                    }
                    wprintw(info_win, "] %d%%", static_cast<int>(progress * 100));

                    int total_seconds = Audio::GetTotalTimeSeconds();
                    int current_seconds = Audio::GetCurrentTimeSeconds();

                    mvwprintw(info_win, progress_y + 1, 1, "%02d:%02d / %02d:%02d", 
                             current_seconds / 60, current_seconds % 60,
                             total_seconds / 60, total_seconds % 60);
                }

                mvwprintw(info_win, max_y - 2, 1, Audio::is_paused ? "PAUSED. Press SPACE to resume." : "Press SPACE to pause.");
            } else {
                mvwprintw(info_win, 2, 1, "No song playing.");
                mvwprintw(info_win, 3, 1, "Press Enter to play selected song.");
                
                // Show ASCII art for selected song even when not playing
                if (!files.empty() && selected_item < files.size()) {
                    std::vector<std::string> asciiArt = AsciiArt::extractAlbumArtASCII(files[selected_item].string());
                    int art_start_y = 5;
                    for (size_t i = 0; i < asciiArt.size() && (art_start_y + i) < (max_y - 3); ++i) {
                        mvwprintw(info_win, art_start_y + i, 1, "%s", asciiArt[i].c_str());
                    }
                }
            }
            
            refresh();
            wrefresh(info_win);
            delwin(info_win);
            
            switch(ch) {
                case KEY_UP:
                    if (selected_item > 0) selected_item--;
                    break;
                case KEY_DOWN:
                    if (selected_item < files.size() - 1) selected_item++;
                    break;
                case 10: // Enter key
                    if (Audio::is_playing) {
                        Audio::StopAudio();
                    }
                    
                    Audio::PlayAudio(files[selected_item].string());
                    Audio::is_playing = true;
                    break;
                case ' ': // Space key to pause/resume
                    if (Audio::is_playing) {
                        Audio::is_paused = !Audio::is_paused;
                    }
                    break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        nodelay(stdscr, FALSE);

        // Clean up audio before exiting
        if (Audio::is_playing) {
            Audio::StopAudio();
        }
    }
}

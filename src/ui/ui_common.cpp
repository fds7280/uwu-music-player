#include "ui/ui_common.h"
#include <cstring>
#include <vector>

namespace UI {
 void init() {
    initscr();
    start_color();
    
    // Check if terminal supports colors
    if (!has_colors()) {
        endwin();
        printf("Your terminal does not support colors\n");
        exit(1);
    }
    
    // Basic 16 colors for ASCII art (matching terminal palette)
    init_pair(0, COLOR_BLACK, COLOR_BLACK);     // Black
    init_pair(1, COLOR_RED, COLOR_BLACK);       // Red
    init_pair(2, COLOR_GREEN, COLOR_BLACK);     // Green
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);    // Yellow
    init_pair(4, COLOR_BLUE, COLOR_BLACK);      // Blue
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);   // Magenta
    init_pair(6, COLOR_CYAN, COLOR_BLACK);      // Cyan
    init_pair(7, COLOR_WHITE, COLOR_BLACK);     // White
    
    // Bright versions (8-15)
    init_pair(8, COLOR_BLACK, COLOR_BLACK);     // Dark gray (using black)
    init_pair(9, COLOR_RED, COLOR_BLACK);       // Bright red
    init_pair(10, COLOR_GREEN, COLOR_BLACK);    // Bright green
    init_pair(11, COLOR_YELLOW, COLOR_BLACK);   // Bright yellow
    init_pair(12, COLOR_BLUE, COLOR_BLACK);     // Bright blue
    init_pair(13, COLOR_MAGENTA, COLOR_BLACK);  // Bright magenta
    init_pair(14, COLOR_CYAN, COLOR_BLACK);     // Bright cyan
    init_pair(15, COLOR_WHITE, COLOR_BLACK);    // Bright white
    
    // UI element colors (20-29)
    init_pair(20, COLOR_YELLOW, COLOR_BLACK);   // Headers
    init_pair(21, COLOR_CYAN, COLOR_BLACK);     // Borders
    init_pair(22, COLOR_GREEN, COLOR_BLACK);    // Success/Playing
    init_pair(23, COLOR_RED, COLOR_BLACK);      // Error/Stopped
    init_pair(24, COLOR_MAGENTA, COLOR_BLACK);  // Highlights
    init_pair(25, COLOR_BLUE, COLOR_BLACK);     // Info text
    init_pair(26, COLOR_WHITE, COLOR_RED);      // Warnings
    init_pair(27, COLOR_BLACK, COLOR_WHITE);    // Inverted
    init_pair(28, COLOR_WHITE, COLOR_BLUE);     // Selection
    init_pair(29, COLOR_YELLOW, COLOR_BLUE);    // Active selection
    
    // Progress bar colors (30-31)
    init_pair(30, COLOR_WHITE, COLOR_GREEN);    // Progress filled
    init_pair(31, COLOR_WHITE, COLOR_BLACK);    // Progress empty
    
    // Special backgrounds (32-35)
    init_pair(32, COLOR_BLACK, COLOR_GREEN);    // Success background
    init_pair(33, COLOR_WHITE, COLOR_RED);      // Error background
    init_pair(34, COLOR_BLACK, COLOR_YELLOW);   // Warning background
    init_pair(35, COLOR_WHITE, COLOR_BLUE);     // Info background
    
    // Set up ncurses options
    noecho();        // Don't echo input characters
    cbreak();        // Disable line buffering
    keypad(stdscr, TRUE);  // Enable special keys
    curs_set(0);     // Hide cursor
    
    // Enable mouse support if available
    mousemask(ALL_MOUSE_EVENTS, NULL);
    
    // Set timeout for getch() in milliseconds (optional)
    // timeout(100);  // Uncomment if you want non-blocking input
 }

  void cleanup() {
        endwin();
    }
    
    void drawBox(WINDOW* win, int y, int x, int height, int width) {
        // Draw colored box
        wattron(win, COLOR_PAIR(1)); // Cyan color for box
        
        mvwaddch(win, y, x, ACS_ULCORNER);
        mvwaddch(win, y, x + width - 1, ACS_URCORNER);
        mvwaddch(win, y + height - 1, x, ACS_LLCORNER);
        mvwaddch(win, y + height - 1, x + width - 1, ACS_LRCORNER);
        
        for (int i = 1; i < width - 1; i++) {
            mvwaddch(win, y, x + i, ACS_HLINE);
            mvwaddch(win, y + height - 1, x + i, ACS_HLINE);
        }
        
        for (int i = 1; i < height - 1; i++) {
            mvwaddch(win, y + i, x, ACS_VLINE);
            mvwaddch(win, y + i, x + width - 1, ACS_VLINE);
        }
        
        wattroff(win, COLOR_PAIR(1));
    }
    
    void centerText(WINDOW* win, int y, const std::string& text) {
        int max_x = getmaxx(win);
        int x = (max_x - text.length()) / 2;
        mvwprintw(win, y, x, "%s", text.c_str());
    }
    
    void showStartupScreen() {
        clear();
        
        // Anime girl ASCII art using only . and : with colors
        std::vector<std::pair<std::string, int>> anime_art = {
            {"                    ........::::........                    ", 5},
            {"                 ..:::::::::::::::::::::..                 ", 5},
            {"               .:::....:::::::::::....:::.                 ", 5},
            {"              ::::..    .::::::::.    ..:::                ", 5},
            {"             ::::.        ::::::.        .:::               ", 5},
            {"            ::::.     ..    :::    ..     .:::              ", 6},
            {"           ::::.     ::::   :::   ::::     .:::             ", 6},
            {"          ::::       ::::   :::   ::::       :::            ", 6},
            {"         ::::.        ::     :     ::        .:::           ", 6},
            {"        ::::.                               .:::            ", 5},
            {"       ::::.           .............           .:::         ", 5},
            {"      ::::.         .::::::::::::::::::.         .:::      ", 5},
            {"     ::::.        .::::::::::::::::::::::::.        .:::   ", 5},
            {"    ::::.       .:::::::................::::.       .:::  ", 5},
            {"   ::::.       :::::.                    .::::       .:::  ", 5},
            {"  ::::.       ::::.                        .:::       .:::.", 5},
            {" ::::.       ::::.                          .:::       .:::", 5},
            {"::::.        :::                              :::        .:::", 5},
            {":::.         :::                              :::         .:::", 5},
            {":::.         :::                              :::         .:::", 5},
            {":::.         ::::.                          .:::         .:::", 5},
            {"::::.        :::::.                        .::::        .:::", 5},
            {" ::::.        :::::..                    ..:::::        .:::", 5},
            {"  ::::.        :::::::................:::::::        .:::  ", 5},
            {"   ::::.        .:::::::::::::::::::::::::::.        .:::  ", 5},
            {"    ::::.         .::::::::::::::::::::::.         .:::    ", 5},
            {"     ::::.           ...............           .:::         ", 5},
            {"      ::::.                                   .:::          ", 5},
            {"       ::::.                                 .:::           ", 5},
            {"        ::::.                               .:::            ", 5},
            {"         ::::.                             .:::             ", 5},
            {"          ::::.                           .:::              ", 5},
            {"           ::::.                         .:::               ", 5},
            {"            ::::.                       .:::                ", 5},
            {"             ::::.                     .:::                 ", 5},
            {"              ::::.                   .:::                  ", 5},
            {"               ::::..               ..:::                   ", 5},
            {"                .::::::::.......::::::::                   ", 5},
            {"                  .:::::::::::::::::.                      ", 5},
            {"                     ............                          ", 5},
            {"", 7},
            {"            ♪ ♫ ♪  UwU Music Player  ♪ ♫ ♪                 ", 2},
            {"                                                            ", 7},
            {"               Press any key to continue...                 ", 3}
        };
        
        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);
        
        // Center the art
        int start_y = (max_y - anime_art.size()) / 2;
        
        for (size_t i = 0; i < anime_art.size(); i++) {
            int start_x = (max_x - anime_art[i].first.length()) / 2;
            attron(COLOR_PAIR(anime_art[i].second));
            mvprintw(start_y + i, start_x, "%s", anime_art[i].first.c_str());
            attroff(COLOR_PAIR(anime_art[i].second));
        }
        
        refresh();
        getch(); // Wait for user input
    }
    
    Mode promptModeSelection() {
        int highlight = 0;
        const char* choices[] = {
            "🎵 Offline Library", 
            "🌐 Online (YouTube)", 
            "📋 Playlist Mode"
        };
        const int num_choices = 3;
        
        while(true) {
            clear();
            
            int max_y, max_x;
            getmaxyx(stdscr, max_y, max_x);
            
            // Draw header with color
            attron(COLOR_PAIR(30)); // Cyan on blue background
            for (int i = 0; i < max_x; i++) {
                mvprintw(0, i, " ");
                mvprintw(1, i, " ");
                mvprintw(2, i, " ");
            }
            centerText(stdscr, 1, "=== UWU MUSIC PLAYER ===");
            attroff(COLOR_PAIR(30));
            
            // Draw title
            attron(COLOR_PAIR(2)); // Yellow
            mvprintw(LINES / 2 - 3, (COLS - 20) / 2, "Select a mode:");
            attroff(COLOR_PAIR(2));
            
            // Draw menu options with colors
            for(int i = 0; i < num_choices; ++i) {
                int y_pos = LINES / 2 + i;
                int x_pos = (COLS - strlen(choices[i])) / 2;
                
                if (i == highlight) {
                    attron(COLOR_PAIR(8)); // Inverted colors for selection
                    mvprintw(y_pos, x_pos - 2, "> ");
                    mvprintw(y_pos, x_pos, "%s", choices[i]);
                    mvprintw(y_pos, x_pos + strlen(choices[i]), " <");
                    attroff(COLOR_PAIR(8));
                } else {
                    // Different color for each option
                    int color = (i == 0) ? 3 : (i == 1) ? 6 : 5; // Green, Blue, Magenta
                    attron(COLOR_PAIR(color));
                    mvprintw(y_pos, x_pos, "%s", choices[i]);
                    attroff(COLOR_PAIR(color));
                }
            }
            
            // Draw instructions
            attron(COLOR_PAIR(1)); // Cyan
            mvprintw(LINES / 2 + num_choices + 2, (COLS - 35) / 2, "Use ↑/↓ arrows and ENTER to select");
            mvprintw(LINES / 2 + num_choices + 3, (COLS - 25) / 2, "Or press 1, 2, or 3");
            attroff(COLOR_PAIR(1));
            
            // Draw footer
            attron(COLOR_PAIR(30));
            for (int i = 0; i < max_x; i++) {
                mvprintw(max_y - 3, i, " ");
                mvprintw(max_y - 2, i, " ");
                mvprintw(max_y - 1, i, " ");
            }
            centerText(stdscr, max_y - 2, "♪ ♫ ♪ Made with love ♪ ♫ ♪");
            attroff(COLOR_PAIR(30));
            
            refresh();

            int ch = getch();
            switch(ch) {
                case KEY_UP:
                    highlight = (highlight > 0) ? highlight - 1 : num_choices - 1;
                    break;
                case KEY_DOWN:
                    highlight = (highlight < num_choices - 1) ? highlight + 1 : 0;
                    break;
                case 10: // Enter
                    return static_cast<Mode>(highlight);
                case '1':
                    return OFFLINE_MODE;
                case '2':
                    return ONLINE_MODE;
                case '3':
                    return PLAYLIST_MODE;
            }
        }
    }
}

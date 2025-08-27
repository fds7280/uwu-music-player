#include "ui/ui_common.h"
#include <cstring>

namespace UI {
    void init() {
        initscr();
        start_color();
        init_pair(1, COLOR_YELLOW, COLOR_BLACK);
        noecho();
        cbreak();
        keypad(stdscr, TRUE);
        curs_set(0);
    }
    
    void cleanup() {
        endwin();
    }
    
    void drawBox(WINDOW* win, int y, int x, int height, int width) {
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
    }
    
    void centerText(WINDOW* win, int y, const std::string& text) {
        int max_x = getmaxx(win);
        int x = (max_x - text.length()) / 2;
        mvwprintw(win, y, x, "%s", text.c_str());
    }
    
    Mode promptModeSelection() {
        int highlight = 0;
        const char* choices[] = {
            "Offline Library", 
            "Online (YouTube)", 
            "Playlist Mode"
        };
        const int num_choices = 3;
        
        while(true) {
            clear();
            
            // Title
            mvprintw(LINES / 2 - 3, (COLS - 20) / 2, "=== UWU MUSIC PLAYER ===");
            mvprintw(LINES / 2 - 1, (COLS - 15) / 2, "Select a mode:");
            
            // Menu options
            for(int i = 0; i < num_choices; ++i) {
                if (i == highlight) attron(A_REVERSE);
                mvprintw(LINES / 2 + i + 1, (COLS - strlen(choices[i])) / 2, "%s", choices[i]);
                if (i == highlight) attroff(A_REVERSE);
            }
            
            // Instructions
            mvprintw(LINES / 2 + num_choices + 3, (COLS - 35) / 2, "Use UP/DOWN arrows and ENTER to select");
            
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

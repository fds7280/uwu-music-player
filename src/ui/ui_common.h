#ifndef UI_COMMON_H
#define UI_COMMON_H

#include <ncurses.h>
#include <string>

namespace UI {
    // Initialize ncurses
    void init();
    void cleanup();
    
    // Common UI elements
    void drawBox(WINDOW* win, int y, int x, int height, int width);
    void centerText(WINDOW* win, int y, const std::string& text);
    
    // Mode selection
    enum Mode {
        OFFLINE_MODE = 0,
        ONLINE_MODE = 1
    };
    
    Mode promptModeSelection();
}

#endif // UI_COMMON_H

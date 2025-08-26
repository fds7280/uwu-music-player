#include "ui/file_browser.h"
#include <ncurses.h>
#include <filesystem>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

namespace UI {
    std::string runFileBrowser(const std::string& start_path) {
        std::string current_path = start_path;
        int highlight = 0;
        std::vector<fs::path> entries;

        while (true) {
            clear();
            printw("Current Directory: %s\n\n", current_path.c_str());
            printw("Use arrow keys to navigate, 's' to select, Enter to enter directory.\n\n");

            entries.clear();
            entries.push_back("..");

            try {
                for (const auto& entry : fs::directory_iterator(current_path)) {
                    entries.push_back(entry.path());
                }
            } catch (const fs::filesystem_error&) {
                return "";
            }
            
            std::sort(entries.begin() + 1, entries.end(), [](const fs::path& a, const fs::path& b) {
                bool a_is_dir = fs::is_directory(a);
                bool b_is_dir = fs::is_directory(b);
                if (a_is_dir != b_is_dir) return a_is_dir > b_is_dir;
                return a.filename().string() < b.filename().string();
            });

            for (size_t i = 0; i < entries.size(); ++i) {
                if (i == highlight) attron(A_REVERSE);
                
                if (fs::is_directory(entries[i])) {
                    attron(COLOR_PAIR(1));
                    printw(" %s\n", entries[i].filename().string().c_str());
                    attroff(COLOR_PAIR(1));
                } else {
                    printw(" %s\n", entries[i].filename().string().c_str());
                }
                
                if (i == highlight) attroff(A_REVERSE);
            }
            refresh();

            int ch = getch();
            switch (ch) {
                case KEY_UP:
                    highlight = (highlight > 0) ? highlight - 1 : entries.size() - 1;
                    break;
                case KEY_DOWN:
                    highlight = (highlight < entries.size() - 1) ? highlight + 1 : 0;
                    break;
                case 10: // Enter
                    if (highlight < entries.size() && fs::is_directory(entries[highlight])) {
                        current_path = fs::canonical(entries[highlight]).string();
                        highlight = 0;
                    }
                    break;
                case 's':
                    if (highlight < entries.size() && fs::is_directory(entries[highlight])) {
                        return fs::canonical(entries[highlight]).string();
                    }
                    break;
                case 27: // Escape
                    return "";
            }
        }
    }
}

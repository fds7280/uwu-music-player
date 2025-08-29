#ifndef SEARCH_UI_H
#define SEARCH_UI_H

#include <string>
#include <vector>
#include "../streaming/youtube_stream.h"
#include "../playlist/playlist_manager.h"
#include <ncurses.h>  // Add this include

namespace UI {
    // Forward declaration for centerText from ui_common.cpp
    void centerText(WINDOW* win, int y, const std::string& text);
    
    // Search functionality
    std::string getSearchQuery(int max_y, int max_x);
    Streaming::SearchResult selectFromResults(const std::vector<Streaming::SearchResult>& results);
    
    // Main modes
    void runOnlineMode();
    void runPlaylistMode();
    
    // Playlist functionality
    void viewSavedPlaylists();
    void viewPlaylist(const Playlist::PlaylistInfo& playlist);
    void playYouTubeVideo(const Streaming::SearchResult& video);
    void playEntirePlaylist(const Playlist::PlaylistInfo& playlist, int start_index);
}

#endif // SEARCH_UI_H

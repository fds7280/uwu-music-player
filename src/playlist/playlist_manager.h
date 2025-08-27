#ifndef PLAYLIST_MANAGER_H
#define PLAYLIST_MANAGER_H

#include <string>
#include <vector>

namespace Playlist {
    struct Song {
        std::string title;
        std::string artist;
        std::string duration;
        std::string id;       // YouTube video ID
    };
    
    struct PlaylistInfo {
        std::string name;
        std::string url;
        std::vector<Song> songs;
    };
    
    // Extract playlist info from YouTube/YouTube Music URL
    PlaylistInfo extractYouTubePlaylist(const std::string& url);
    
    // Save/Load playlists
    void savePlaylist(const PlaylistInfo& playlist, const std::string& filename);
    PlaylistInfo loadPlaylist(const std::string& filename);
    std::vector<std::string> listSavedPlaylists();
}

#endif

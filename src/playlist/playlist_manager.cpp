#include "playlist/playlist_manager.h"
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include "../utils/utils.h"

namespace Playlist {
    
    PlaylistInfo extractYouTubePlaylist(const std::string& url) {
        PlaylistInfo playlist;
        playlist.url = url;
        
        // Extract playlist info using yt-dlp
        std::string temp_file = "/tmp/youtube_playlist_" + std::to_string(time(nullptr)) + ".txt";
        
        // First get playlist title
        std::string title_cmd = "yt-dlp --flat-playlist --print \"%(playlist_title)s\" \"" 
                               + url + "\" 2>/dev/null | head -1 > " + temp_file;
        system(title_cmd.c_str());
        
        std::ifstream title_file(temp_file);
        std::getline(title_file, playlist.name);
        title_file.close();
        
        // Then get all video info
        std::string cmd = "yt-dlp --flat-playlist --print \"%(title)s|%(uploader)s|%(duration_string)s|%(id)s\" \"" 
                         + url + "\" > " + temp_file + " 2>/dev/null";
        
        system(cmd.c_str());
        
        // Read the results
        std::ifstream file(temp_file);
        std::string line;
        
        while (std::getline(file, line)) {
            // Parse video info
            size_t pos1 = line.find('|');
            size_t pos2 = line.find('|', pos1 + 1);
            size_t pos3 = line.find('|', pos2 + 1);
            
            if (pos1 != std::string::npos && pos2 != std::string::npos && pos3 != std::string::npos) {
                Song song;
                song.title = line.substr(0, pos1);
                song.artist = line.substr(pos1 + 1, pos2 - pos1 - 1);
                song.duration = line.substr(pos2 + 1, pos3 - pos2 - 1);
                song.id = line.substr(pos3 + 1);
                
                // Clean up duration (sometimes it's "NA" or empty)
                if (song.duration == "NA" || song.duration.empty()) {
                    song.duration = "0:00";
                }
                
                playlist.songs.push_back(song);
            }
        }
        
        file.close();
        std::filesystem::remove(temp_file);
        
        // If playlist name is empty, use a default
        if (playlist.name.empty()) {
            playlist.name = "YouTube Playlist";
        }
        
        return playlist;
    }
    
    void savePlaylist(const PlaylistInfo& playlist, const std::string& filename) {
        std::string playlist_dir = Utils::getCacheDirectory() + "/playlists/";
        std::filesystem::create_directories(playlist_dir);
        
        std::ofstream file(playlist_dir + filename);
        file << playlist.name << "\n";
        file << playlist.url << "\n";
        file << playlist.songs.size() << "\n";
        
        for (const auto& song : playlist.songs) {
            file << song.title << "|" << song.artist << "|" 
                 << song.duration << "|" << song.id << "\n";
        }
        
        file.close();
    }
    
    PlaylistInfo loadPlaylist(const std::string& filename) {
        PlaylistInfo playlist;
        std::string playlist_dir = Utils::getCacheDirectory() + "/playlists/";
        std::ifstream file(playlist_dir + filename);
        
        if (!file) return playlist;
        
        std::string line;
        std::getline(file, playlist.name);
        std::getline(file, playlist.url);
        
        int song_count;
        std::getline(file, line);
        song_count = std::stoi(line);
        
        for (int i = 0; i < song_count; i++) {
            std::getline(file, line);
            size_t pos1 = line.find('|');
            size_t pos2 = line.find('|', pos1 + 1);
            size_t pos3 = line.find('|', pos2 + 1);
            
            if (pos1 != std::string::npos && pos2 != std::string::npos && pos3 != std::string::npos) {
                Song song;
                song.title = line.substr(0, pos1);
                song.artist = line.substr(pos1 + 1, pos2 - pos1 - 1);
                song.duration = line.substr(pos2 + 1, pos3 - pos2 - 1);
                song.id = line.substr(pos3 + 1);
                
                playlist.songs.push_back(song);
            }
        }
        
        file.close();
        return playlist;
    }
    
    std::vector<std::string> listSavedPlaylists() {
        std::vector<std::string> playlists;
        std::string playlist_dir = Utils::getCacheDirectory() + "/playlists/";
        
        if (std::filesystem::exists(playlist_dir)) {
            for (const auto& entry : std::filesystem::directory_iterator(playlist_dir)) {
                if (entry.is_regular_file() && entry.path().extension() == ".playlist") {
                    playlists.push_back(entry.path().filename().string());
                }
            }
        }
        
        return playlists;
    }
}

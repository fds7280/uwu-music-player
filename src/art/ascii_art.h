#ifndef ASCII_ART_H
#define ASCII_ART_H

#include <string>
#include <vector>

namespace AsciiArt {
    const std::string ASCII_CHARS = " .:-=+*#%@";
    const int THUMBNAIL_WIDTH = 60;
    const int THUMBNAIL_HEIGHT = 20;
    
    struct ColoredChar {
        char character;
        int color_pair;
    };
    
    // Generate ASCII art from raw image data
    std::vector<std::string> generateASCIIArt(const char* imageData, size_t dataSize);
    
    // Generate colored ASCII art
    std::vector<std::vector<ColoredChar>> generateColoredASCII(const char* imageData, size_t dataSize);
    
    // Extract album art from MP3 file and convert to ASCII
    std::vector<std::string> extractAlbumArtASCII(const std::string& filePath);
    
    // Get YouTube thumbnail and convert to ASCII
    std::vector<std::string> getYouTubeThumbnailASCII(const std::string& video_id);
    
    // Get colored YouTube thumbnail
    std::vector<std::vector<ColoredChar>> getYouTubeColoredThumbnail(const std::string& video_id);
}

#endif // ASCII_ART_H

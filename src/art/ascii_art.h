#ifndef ASCII_ART_H
#define ASCII_ART_H

#include <string>
#include <vector>

namespace AsciiArt {
    // Much higher resolution for pixel-like appearance
    const int THUMBNAIL_WIDTH = 120;   // Double width
    const int THUMBNAIL_HEIGHT = 40;   // Double height
    
    struct ColoredChar {
        char character;
        int color_pair;
    };
    
    // Generate colored ASCII art
    std::vector<std::vector<ColoredChar>> generateColoredASCII(const char* imageData, size_t dataSize);
    
    // Generate ASCII art from raw image data
    std::vector<std::string> generateASCIIArt(const char* imageData, size_t dataSize);
    
    // Extract album art from MP3 file and convert to ASCII
    std::vector<std::string> extractAlbumArtASCII(const std::string& filePath);
    
    // Get colored YouTube thumbnail
    std::vector<std::vector<ColoredChar>> getYouTubeColoredThumbnail(const std::string& video_id);
    
    // Get YouTube thumbnail and convert to ASCII
    std::vector<std::string> getYouTubeThumbnailASCII(const std::string& video_id);
}

#endif // ASCII_ART_H

#ifndef ASCII_ART_H
#define ASCII_ART_H

#include <string>
#include <vector>

namespace AsciiArt {
    const std::string ASCII_CHARS = " .:-=+*#%@";
    const int THUMBNAIL_WIDTH = 40;
    const int THUMBNAIL_HEIGHT = 20;
    
    // Generate ASCII art from raw image data
    std::vector<std::string> generateASCIIArt(const char* imageData, size_t dataSize);
    
    // Extract album art from MP3 file and convert to ASCII
    std::vector<std::string> extractAlbumArtASCII(const std::string& filePath);
}

#endif // ASCII_ART_H

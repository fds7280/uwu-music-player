#ifndef ASCII_ART_H
#define ASCII_ART_H

#include <string>
#include <vector>

namespace AsciiArt {
    // Ultra-detailed ASCII character set (sorted by visual density)
    const std::string ASCII_CHARS_ULTRA = " `.-':_,^=;><+!rc*/z?sLTv)J7(|Fi{C}fI31tlu[neoZ5Yxjya]2ESwqkP6h9d4VpOGbUAKXHm8RD#$Bg0MNWQ%&@";
    
    // Even higher resolution for maximum detail
    const int THUMBNAIL_WIDTH = 120;   // Triple width
    const int THUMBNAIL_HEIGHT = 60;   // Triple height
    
    // Color mode for terminals that support it
    const bool USE_COLOR = true;
    
    // Generate ASCII art from raw image data
    std::vector<std::string> generateASCIIArt(const char* imageData, size_t dataSize);
    
    // Ultra-enhanced ASCII generation with advanced processing
    std::vector<std::string> generateUltraASCII(const char* imageData, size_t dataSize);
    
    // Extract album art from MP3 file and convert to ASCII
    std::vector<std::string> extractAlbumArtASCII(const std::string& filePath);
    
    // Get YouTube thumbnail and convert to ASCII
    std::vector<std::string> getYouTubeThumbnailASCII(const std::string& video_id);
}

#endif // ASCII_ART_H

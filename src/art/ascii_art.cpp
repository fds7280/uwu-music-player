#include "art/ascii_art.h" 
#include <algorithm>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/id3v2frame.h>
#include <taglib/attachedpictureframe.h>
#include <filesystem>
#include <cstdlib>
#include <fstream>
#include <cmath>

namespace AsciiArt {
    
    // Better color mapping to terminal colors
    int rgbToTerminalColor(int r, int g, int b) {
        // Terminal color palette (basic 16 colors)
        // 0=black, 1=red, 2=green, 3=yellow, 4=blue, 5=magenta, 6=cyan, 7=white
        // 8-15 are bright versions
        
        // Normalize RGB values
        float rf = r / 255.0f;
        float gf = g / 255.0f;
        float bf = b / 255.0f;
        
        // Calculate brightness
        float brightness = (rf + gf + bf) / 3.0f;
        
        // Very dark pixels
        if (brightness < 0.1) return 0; // Black
        
        // Determine dominant color channel
        float max_val = std::max({rf, gf, bf});
        float min_val = std::min({rf, gf, bf});
        float saturation = (max_val - min_val);
        
        // Grayscale (low saturation)
        if (saturation < 0.15) {
            if (brightness < 0.3) return 8;  // Dark gray
            if (brightness < 0.6) return 7;  // Light gray
            return 15; // White
        }
        
        // Colored pixels - find closest terminal color
        if (rf > gf && rf > bf) {
            // Red dominant
            if (gf > bf * 1.5) return (brightness > 0.5) ? 11 : 3; // Yellow
            return (brightness > 0.5) ? 9 : 1; // Red
        } else if (gf > rf && gf > bf) {
            // Green dominant
            if (bf > rf * 1.5) return (brightness > 0.5) ? 14 : 6; // Cyan
            return (brightness > 0.5) ? 10 : 2; // Green
        } else if (bf > rf && bf > gf) {
            // Blue dominant
            if (rf > gf * 1.5) return (brightness > 0.5) ? 13 : 5; // Magenta
            if (gf > rf * 1.5) return (brightness > 0.5) ? 14 : 6; // Cyan
            return (brightness > 0.5) ? 12 : 4; // Blue
        }
        
        return 7; // Default white
    }
    
    std::vector<std::vector<ColoredChar>> generateColoredASCII(const char* imageData, size_t dataSize) {
        std::vector<std::vector<ColoredChar>> coloredArt(THUMBNAIL_HEIGHT, 
            std::vector<ColoredChar>(THUMBNAIL_WIDTH, {'.', 7}));
        
        if (!imageData || dataSize == 0) {
            // No image pattern
            for (int y = 0; y < THUMBNAIL_HEIGHT; y++) {
                for (int x = 0; x < THUMBNAIL_WIDTH; x++) {
                    if (y == 0 || y == THUMBNAIL_HEIGHT-1 || x == 0 || x == THUMBNAIL_WIDTH-1) {
                        coloredArt[y][x] = {':', 6}; // Cyan border
                    } else if (y == THUMBNAIL_HEIGHT/2 && x > 20 && x < THUMBNAIL_WIDTH-20) {
                        const char* text = "NO THUMBNAIL";
                        int text_start = (THUMBNAIL_WIDTH - 12) / 2;
                        if (x >= text_start && x < text_start + 12) {
                            coloredArt[y][x] = {text[x - text_start], 3}; // Yellow text
                        }
                    } else {
                        coloredArt[y][x] = {'.', 8}; // Gray dots
                    }
                }
            }
            return coloredArt;
        }
        
        // Use dots and colons but packed tightly for pixel effect
        // More dots = darker, colons = brighter
        const char* pixels = ".:";
        
        for (int y = 0; y < THUMBNAIL_HEIGHT; y++) {
            for (int x = 0; x < THUMBNAIL_WIDTH; x++) {
                size_t index = (y * THUMBNAIL_WIDTH + x) * 3;
                
                if (index + 2 < dataSize) {
                    unsigned char r = static_cast<unsigned char>(imageData[index]);
                    unsigned char g = static_cast<unsigned char>(imageData[index + 1]);
                    unsigned char b = static_cast<unsigned char>(imageData[index + 2]);
                    
                    // Calculate brightness for character selection
                    int brightness = (r * 299 + g * 587 + b * 114) / 1000;
                    
                    // Use dot for dark pixels, colon for bright pixels
                    char ch = (brightness < 128) ? '.' : ':';
                    
                    // Get appropriate color based on actual RGB values
                    int color = rgbToTerminalColor(r, g, b);
                    
                    coloredArt[y][x] = {ch, color};
                } else {
                    coloredArt[y][x] = {' ', 7};
                }
            }
        }
        
        return coloredArt;
    }
    
    std::vector<std::string> generateASCIIArt(const char* imageData, size_t dataSize) {
        auto colored = generateColoredASCII(imageData, dataSize);
        std::vector<std::string> result(THUMBNAIL_HEIGHT, std::string(THUMBNAIL_WIDTH, ' '));
        
        for (int y = 0; y < THUMBNAIL_HEIGHT; y++) {
            for (int x = 0; x < THUMBNAIL_WIDTH; x++) {
                result[y][x] = colored[y][x].character;
            }
        }
        return result;
    }
    
    std::vector<std::string> extractAlbumArtASCII(const std::string& filePath) {
        TagLib::MPEG::File file(filePath.c_str());
        
        if (!file.isValid()) {
            return generateASCIIArt(nullptr, 0);
        }
        
        TagLib::ID3v2::Tag* id3v2Tag = file.ID3v2Tag();
        if (!id3v2Tag) {
            return generateASCIIArt(nullptr, 0);
        }
        
        TagLib::ID3v2::FrameList frameList = id3v2Tag->frameList("APIC");
        if (frameList.isEmpty()) {
            return generateASCIIArt(nullptr, 0);
        }
        
        TagLib::ID3v2::AttachedPictureFrame* pictureFrame = 
            static_cast<TagLib::ID3v2::AttachedPictureFrame*>(frameList.front());
        
        if (!pictureFrame) {
            return generateASCIIArt(nullptr, 0);
        }
        
        TagLib::ByteVector imageData = pictureFrame->picture();
        return generateASCIIArt(imageData.data(), imageData.size());
    }
    
    std::vector<std::vector<ColoredChar>> getYouTubeColoredThumbnail(const std::string& video_id) {
        std::string temp_dir = "/tmp/uwu_thumbnails/";
        std::string temp_jpg = temp_dir + video_id + ".jpg";
        std::string temp_raw = temp_dir + video_id + ".rgb";
        
        // Create temp directory if it doesn't exist
        std::filesystem::create_directories(temp_dir);
        
        // Try different thumbnail qualities
        std::vector<std::string> thumbnail_urls = {
            "https://img.youtube.com/vi/" + video_id + "/maxresdefault.jpg",
            "https://img.youtube.com/vi/" + video_id + "/sddefault.jpg",
            "https://img.youtube.com/vi/" + video_id + "/hqdefault.jpg",
            "https://img.youtube.com/vi/" + video_id + "/mqdefault.jpg"
        };
        
        bool downloaded = false;
        for (const auto& url : thumbnail_urls) {
            std::string download_cmd = "curl -s -o \"" + temp_jpg + "\" \"" + url + "\"";
            if (system(download_cmd.c_str()) == 0) {
                if (std::filesystem::exists(temp_jpg) && std::filesystem::file_size(temp_jpg) > 1000) {
                    downloaded = true;
                    break;
                }
            }
        }
        
        if (!downloaded) {
            return generateColoredASCII(nullptr, 0);
        }
        
        // Convert JPEG to raw RGB data using ImageMagick with better quality
        std::string convert_cmd = "magick convert \"" + temp_jpg + 
                                  "\" -colorspace sRGB -resize " + 
                                  std::to_string(THUMBNAIL_WIDTH) + "x" + 
                                  std::to_string(THUMBNAIL_HEIGHT) + 
                                  "! -depth 8 rgb:\"" + temp_raw + "\" 2>/dev/null";
        
        if (system(convert_cmd.c_str()) != 0) {
            std::filesystem::remove(temp_jpg);
            return generateColoredASCII(nullptr, 0);
        }
        
        // Read the raw RGB data
        std::ifstream file(temp_raw, std::ios::binary);
        if (!file) {
            std::filesystem::remove(temp_jpg);
            std::filesystem::remove(temp_raw);
            return generateColoredASCII(nullptr, 0);
        }
        
        file.seekg(0, std::ios::end);
        size_t file_size = file.tellg();
        file.seekg(0, std::ios::beg);
        
        std::vector<char> image_data(file_size);
        file.read(image_data.data(), file_size);
        file.close();
        
        // Generate colored ASCII art
        auto colored_art = generateColoredASCII(image_data.data(), file_size);
        
        // Clean up temp files
        std::filesystem::remove(temp_jpg);
        std::filesystem::remove(temp_raw);
        
        return colored_art;
    }
    
    std::vector<std::string> getYouTubeThumbnailASCII(const std::string& video_id) {
        auto colored = getYouTubeColoredThumbnail(video_id);
        std::vector<std::string> result(THUMBNAIL_HEIGHT, std::string(THUMBNAIL_WIDTH, ' '));
        
        for (int y = 0; y < THUMBNAIL_HEIGHT; y++) {
            for (int x = 0; x < THUMBNAIL_WIDTH; x++) {
                result[y][x] = colored[y][x].character;
            }
        }
        return result;
    }
}

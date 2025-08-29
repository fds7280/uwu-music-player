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

namespace AsciiArt {
    
    std::vector<std::vector<ColoredChar>> generateColoredASCII(const char* imageData, size_t dataSize) {
        std::vector<std::vector<ColoredChar>> coloredArt(THUMBNAIL_HEIGHT, 
            std::vector<ColoredChar>(THUMBNAIL_WIDTH, {' ', 7}));
        
        if (!imageData || dataSize == 0) {
            // No image pattern with colors
            std::vector<std::string> noImagePattern = {
                ":::::::::::::::::::::::::::::::::::::::::::::::::::::::::",
                ":                                                       :",
                ":                NO THUMBNAIL FOUND                     :",
                ":                                                       :",
                ":   ................................................    :",
                ":   .                                          .    :",
                ":   .          : . : . : . : . : . :           .    :",
                ":   .                                          .    :",
                ":   .             YOUTUBE MUSIC                .    :",
                ":   .                                          .    :",
                ":   .          : . : . : . : . : . :           .    :",
                ":   .                                          .    :",
                ":   ................................................    :",
                ":                                                       :",
                ":                Loading thumbnail...                   :",
                ":                                                       :",
                ":::::::::::::::::::::::::::::::::::::::::::::::::::::::::"
            };
            
            for (int y = 0; y < THUMBNAIL_HEIGHT && y < noImagePattern.size(); y++) {
                std::string line = noImagePattern[y];
                for (int x = 0; x < THUMBNAIL_WIDTH && x < line.length(); x++) {
                    char ch = line[x];
                    int color = 7; // Default white
                    
                    if (ch == ':') {
                        color = 1; // Cyan for border
                    } else if (ch == '.') {
                        color = 6; // Blue for dots
                    }
                    
                    coloredArt[y][x] = {ch, color};
                }
            }
            
            return coloredArt;
        }
        
        // Process image with colors based on brightness
        const std::string chars = " ..:::::";
        
        for (int y = 0; y < THUMBNAIL_HEIGHT; y++) {
            for (int x = 0; x < THUMBNAIL_WIDTH; x++) {
                size_t index = (y * THUMBNAIL_WIDTH + x) * 3;
                
                if (index + 2 < dataSize) {
                    unsigned char r = static_cast<unsigned char>(imageData[index]);
                    unsigned char g = static_cast<unsigned char>(imageData[index + 1]);
                    unsigned char b = static_cast<unsigned char>(imageData[index + 2]);
                    
                    // Convert to grayscale
                    int brightness = (r * 299 + g * 587 + b * 114) / 1000;
                    
                    // Select character based on brightness
                    int charIndex = (brightness * (chars.length() - 1)) / 255;
                    charIndex = std::max(0, std::min((int)chars.length() - 1, charIndex));
                    
                    // Select color based on RGB values
                    int color_pair = 7; // Default white
                    
                    // Determine dominant color
                    if (brightness < 30) {
                        color_pair = 0; // Black
                    } else if (r > g && r > b && r > 100) {
                        // Red dominant
                        if (brightness > 180) color_pair = 3; // Yellow
                        else color_pair = 1; // Red
                    } else if (g > r && g > b && g > 100) {
                        // Green dominant
                        color_pair = 2; // Green
                    } else if (b > r && b > g && b > 100) {
                        // Blue dominant
                        if (brightness > 180) color_pair = 6; // Cyan
                        else color_pair = 4; // Blue
                    } else {
                        // Grayscale
                        if (brightness < 85) color_pair = 8; // Dark gray
                        else if (brightness < 170) color_pair = 7; // Light gray
                        else color_pair = 15; // White
                    }
                    
                    coloredArt[y][x] = {chars[charIndex], color_pair};
                } else {
                    coloredArt[y][x] = {' ', 7};
                }
            }
        }
        
        // Add colored border
        for (int x = 0; x < THUMBNAIL_WIDTH; x++) {
            coloredArt[0][x] = {':', 6}; // Cyan border
            coloredArt[THUMBNAIL_HEIGHT-1][x] = {':', 6};
        }
        for (int y = 0; y < THUMBNAIL_HEIGHT; y++) {
            coloredArt[y][0] = {':', 6};
            coloredArt[y][THUMBNAIL_WIDTH-1] = {':', 6};
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
        
        // Convert JPEG to raw RGB data using ImageMagick
        std::string convert_cmd = "magick convert \"" + temp_jpg + "\" -resize " + 
                                  std::to_string(THUMBNAIL_WIDTH) + "x" + 
                                  std::to_string(THUMBNAIL_HEIGHT) + "! " +
                                  "-depth 8 rgb:\"" + temp_raw + "\" 2>/dev/null";
        
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

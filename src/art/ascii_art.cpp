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
    
    std::vector<std::string> generateASCIIArt(const char* imageData, size_t dataSize) {
        std::vector<std::string> asciiArt(THUMBNAIL_HEIGHT, std::string(THUMBNAIL_WIDTH, ' '));
        
        if (!imageData || dataSize == 0) {
            // Clean "no image" display
            std::vector<std::string> noImagePattern = {
                "╔══════════════════════════════════════════════════════════════════════════════════════╗",
                "║                                                                                      ║",
                "║                                    🎵 YOUTUBE 🎵                                     ║",
                "║                                                                                      ║",
                "║    ██████████████████████████████████████████████████████████████████████████      ║",
                "║    █                                                                        █      ║",
                "║    █                                                                        █      ║",
                "║    █                          No Thumbnail Available                       █      ║",
                "║    █                                                                        █      ║",
                "║    █                             Loading...                                █      ║",
                "║    █                                                                        █      ║",
                "║    █                                                                        █      ║",
                "║    ██████████████████████████████████████████████████████████████████████████      ║",
                "║                                                                                      ║",
                "║                                  ♪ ♫ ♪ ♫ ♪ ♫ ♪                                    ║",
                "║                                                                                      ║",
                "╚══════════════════════════════════════════════════════════════════════════════════════╝"
            };
            
            // Ensure proper sizing
            for (int i = 0; i < THUMBNAIL_HEIGHT; i++) {
                if (i < noImagePattern.size()) {
                    std::string line = noImagePattern[i];
                    if (line.length() > THUMBNAIL_WIDTH) {
                        asciiArt[i] = line.substr(0, THUMBNAIL_WIDTH);
                    } else {
                        asciiArt[i] = line + std::string(THUMBNAIL_WIDTH - line.length(), ' ');
                    }
                } else {
                    asciiArt[i] = std::string(THUMBNAIL_WIDTH, ' ');
                }
            }
            return asciiArt;
        }
        
        // Simple brightness-based ASCII conversion
        const std::string chars = " .:-=+*#%@";
        
        for (int y = 0; y < THUMBNAIL_HEIGHT; y++) {
            for (int x = 0; x < THUMBNAIL_WIDTH; x++) {
                // Sample the image data
                size_t index = (y * THUMBNAIL_WIDTH + x) * 3;
                
                if (index + 2 < dataSize) {
                    unsigned char r = static_cast<unsigned char>(imageData[index]);
                    unsigned char g = static_cast<unsigned char>(imageData[index + 1]);
                    unsigned char b = static_cast<unsigned char>(imageData[index + 2]);
                    
                    // Convert to grayscale
                    int brightness = (r * 299 + g * 587 + b * 114) / 1000;
                    
                    // Map to ASCII character
                    int charIndex = (brightness * (chars.length() - 1)) / 255;
                    charIndex = std::max(0, std::min((int)chars.length() - 1, charIndex));
                    
                    asciiArt[y][x] = chars[charIndex];
                } else {
                    asciiArt[y][x] = ' ';
                }
            }
        }
        
        return asciiArt;
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
    
    std::vector<std::string> getYouTubeThumbnailASCII(const std::string& video_id) {
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
            return generateASCIIArt(nullptr, 0);
        }
        
        // Convert JPEG to raw RGB data using ImageMagick
        std::string convert_cmd = "magick convert \"" + temp_jpg + "\" -resize " + 
                                  std::to_string(THUMBNAIL_WIDTH) + "x" + 
                                  std::to_string(THUMBNAIL_HEIGHT) + "! " +
                                  "-depth 8 rgb:\"" + temp_raw + "\" 2>/dev/null";
        
        if (system(convert_cmd.c_str()) != 0) {
            std::filesystem::remove(temp_jpg);
            return generateASCIIArt(nullptr, 0);
        }
        
        // Read the raw RGB data
        std::ifstream file(temp_raw, std::ios::binary);
        if (!file) {
            std::filesystem::remove(temp_jpg);
            std::filesystem::remove(temp_raw);
            return generateASCIIArt(nullptr, 0);
        }
        
        file.seekg(0, std::ios::end);
        size_t file_size = file.tellg();
        file.seekg(0, std::ios::beg);
        
        std::vector<char> image_data(file_size);
        file.read(image_data.data(), file_size);
        file.close();
        
        // Generate simple ASCII art
        std::vector<std::string> ascii_art = generateASCIIArt(image_data.data(), file_size);
        
        // Clean up temp files
        std::filesystem::remove(temp_jpg);
        std::filesystem::remove(temp_raw);
        
        return ascii_art;
    }
}

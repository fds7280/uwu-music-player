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
    std::vector<std::string> generateASCIIArt(const char* imageData, size_t dataSize) {
        std::vector<std::string> asciiArt(THUMBNAIL_HEIGHT, std::string(THUMBNAIL_WIDTH, ' '));
        
        if (!imageData || dataSize == 0) {
            // Generate a clean musical themed pattern
            std::vector<std::string> musicPattern = {
                "+--------------------------------------+",
                "|            ALBUM ARTWORK             |",
                "|               ~ * ~                  |",
                "|          +-------------+             |",
                "|          |  *       ~  |             |",
                "|          |             |             |",
                "|          |    ~   *    |             |",
                "|          |             |             |",
                "|          |  *       ~  |             |",
                "|          +-------------+             |",
                "|                                      |",
                "|         ##############               |",
                "|         ##          ##               |",
                "|         ##    *~    ##               |",
                "|         ##          ##               |",
                "|         ##############               |",
                "|                                      |",
                "|            NO IMAGE FOUND            |",
                "+--------------------------------------+",
                "                                        "
            };
            
            for (int i = 0; i < THUMBNAIL_HEIGHT && i < musicPattern.size(); i++) {
                std::string line = musicPattern[i];
                if (line.length() > THUMBNAIL_WIDTH) {
                    asciiArt[i] = line.substr(0, THUMBNAIL_WIDTH);
                } else {
                    asciiArt[i] = line + std::string(THUMBNAIL_WIDTH - line.length(), ' ');
                }
            }
            return asciiArt;
        }
        
        // Image processing algorithm
        std::vector<std::vector<int>> brightness(THUMBNAIL_HEIGHT, std::vector<int>(THUMBNAIL_WIDTH, 0));
        
        for (int y = 0; y < THUMBNAIL_HEIGHT; y++) {
            for (int x = 0; x < THUMBNAIL_WIDTH; x++) {
                int totalBrightness = 0;
                int sampleCount = 0;
                
                for (int sy = -1; sy <= 1; sy++) {
                    for (int sx = -1; sx <= 1; sx++) {
                        int sampleY = y + sy;
                        int sampleX = x + sx;
                        
                        if (sampleY >= 0 && sampleY < THUMBNAIL_HEIGHT && 
                            sampleX >= 0 && sampleX < THUMBNAIL_WIDTH) {
                            
                            size_t index = ((sampleY * THUMBNAIL_WIDTH + sampleX) * dataSize) / 
                                         (THUMBNAIL_WIDTH * THUMBNAIL_HEIGHT);
                            
                            if (index < dataSize) {
                                unsigned char byte1 = static_cast<unsigned char>(imageData[index]);
                                unsigned char byte2 = static_cast<unsigned char>(imageData[(index + 1) % dataSize]);
                                unsigned char byte3 = static_cast<unsigned char>(imageData[(index + 2) % dataSize]);
                                
                                int pixelBrightness = (byte1 * 299 + byte2 * 587 + byte3 * 114) / 1000;
                                totalBrightness += pixelBrightness;
                                sampleCount++;
                            }
                        }
                    }
                }
                
                if (sampleCount > 0) {
                    brightness[y][x] = totalBrightness / sampleCount;
                }
            }
        }
        
        // Apply smoothing and convert to ASCII
        for (int y = 1; y < THUMBNAIL_HEIGHT - 1; y++) {
            for (int x = 1; x < THUMBNAIL_WIDTH - 1; x++) {
                int smoothed = (brightness[y-1][x-1] + brightness[y-1][x] + brightness[y-1][x+1] +
                               brightness[y][x-1]   + brightness[y][x] * 2 + brightness[y][x+1] +
                               brightness[y+1][x-1] + brightness[y+1][x] + brightness[y+1][x+1]) / 10;
                
                int charIndex = (smoothed * (ASCII_CHARS.length() - 1)) / 255;
                charIndex = std::max(0, std::min((int)ASCII_CHARS.length() - 1, charIndex));
                asciiArt[y][x] = ASCII_CHARS[charIndex];
            }
        }
        
        // Add border
        for (int x = 0; x < THUMBNAIL_WIDTH; x++) {
            if (x == 0 || x == THUMBNAIL_WIDTH - 1) {
                for (int y = 0; y < THUMBNAIL_HEIGHT; y++) {
                    asciiArt[y][x] = '|';
                }
            }
        }
        for (int y = 0; y < THUMBNAIL_HEIGHT; y++) {
            if (y == 0 || y == THUMBNAIL_HEIGHT - 1) {
                for (int x = 0; x < THUMBNAIL_WIDTH; x++) {
                    if (x == 0 || x == THUMBNAIL_WIDTH - 1) {
                        asciiArt[y][x] = '+';
                    } else {
                        asciiArt[y][x] = '-';
                    }
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
    std::string convert_cmd = "convert \"" + temp_jpg + "\" -resize " + 
                              std::to_string(THUMBNAIL_WIDTH) + "x" + 
                              std::to_string(THUMBNAIL_HEIGHT) + "! " +
                              "-depth 8 rgb:\"" + temp_raw + "\"";
    
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
    
    // Generate ASCII art
    std::vector<std::string> ascii_art(THUMBNAIL_HEIGHT, std::string(THUMBNAIL_WIDTH, ' '));
    
    // Process the raw RGB data
    for (int y = 0; y < THUMBNAIL_HEIGHT; y++) {
        for (int x = 0; x < THUMBNAIL_WIDTH; x++) {
            int idx = (y * THUMBNAIL_WIDTH + x) * 3;
            if (idx + 2 < image_data.size()) {
                unsigned char r = image_data[idx];
                unsigned char g = image_data[idx + 1];
                unsigned char b = image_data[idx + 2];
                
                // Convert to grayscale
                int brightness = (r * 299 + g * 587 + b * 114) / 1000;
                
                // Map to ASCII character
                int charIndex = (brightness * (ASCII_CHARS.length() - 1)) / 255;
                ascii_art[y][x] = ASCII_CHARS[charIndex];
            }
        }
    }
    
    // Add border
    for (int x = 0; x < THUMBNAIL_WIDTH; x++) {
        ascii_art[0][x] = (x == 0 || x == THUMBNAIL_WIDTH - 1) ? '+' : '-';
        ascii_art[THUMBNAIL_HEIGHT - 1][x] = (x == 0 || x == THUMBNAIL_WIDTH - 1) ? '+' : '-';
    }
    for (int y = 1; y < THUMBNAIL_HEIGHT - 1; y++) {
        ascii_art[y][0] = '|';
        ascii_art[y][THUMBNAIL_WIDTH - 1] = '|';
    }
    
    // Clean up temp files
    std::filesystem::remove(temp_jpg);
    std::filesystem::remove(temp_raw);
    
    return ascii_art;
   }
}

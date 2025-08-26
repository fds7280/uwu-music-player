#include "art/ascii_art.h" 
#include <algorithm>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/id3v2frame.h>
#include <taglib/attachedpictureframe.h>

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
}

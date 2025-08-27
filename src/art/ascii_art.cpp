#include "art/ascii_art.h" 
#include <algorithm>
#include <map>
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
#include <vector>
#include <numeric>

namespace AsciiArt {
    
    // Structure to hold pixel data
    struct Pixel {
        int r, g, b;
        int brightness;
        double detail;
    };
    
    // Advanced gamma correction
    int gammaCorrect(int value, double gamma = 2.2) {
        double normalized = value / 255.0;
        double corrected = std::pow(normalized, 1.0 / gamma);
        return static_cast<int>(corrected * 255);
    }
    
    // Histogram equalization for better contrast
    std::vector<std::vector<int>> equalizeHistogram(const std::vector<std::vector<int>>& image) {
        std::vector<int> histogram(256, 0);
        int totalPixels = image.size() * image[0].size();
        
        // Build histogram
        for (const auto& row : image) {
            for (int pixel : row) {
                histogram[pixel]++;
            }
        }
        
        // Build cumulative distribution
        std::vector<int> cdf(256, 0);
        cdf[0] = histogram[0];
        for (int i = 1; i < 256; i++) {
            cdf[i] = cdf[i-1] + histogram[i];
        }
        
        // Normalize and create lookup table
        std::vector<int> lookupTable(256);
        for (int i = 0; i < 256; i++) {
            lookupTable[i] = static_cast<int>((cdf[i] * 255.0) / totalPixels);
        }
        
        // Apply equalization
        std::vector<std::vector<int>> equalized = image;
        for (int y = 0; y < image.size(); y++) {
            for (int x = 0; x < image[0].size(); x++) {
                equalized[y][x] = lookupTable[image[y][x]];
            }
        }
        
        return equalized;
    }
    
    // Adaptive local contrast enhancement
    std::vector<std::vector<double>> enhanceLocalContrast(const std::vector<std::vector<int>>& image) {
        int height = image.size();
        int width = image[0].size();
        std::vector<std::vector<double>> enhanced(height, std::vector<double>(width, 0));
        
        int windowSize = 5;
        
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                // Calculate local statistics
                double sum = 0, sumSq = 0;
                int count = 0;
                
                for (int dy = -windowSize; dy <= windowSize; dy++) {
                    for (int dx = -windowSize; dx <= windowSize; dx++) {
                        int ny = y + dy;
                        int nx = x + dx;
                        
                        if (ny >= 0 && ny < height && nx >= 0 && nx < width) {
                            sum += image[ny][nx];
                            sumSq += image[ny][nx] * image[ny][nx];
                            count++;
                        }
                    }
                }
                
                double mean = sum / count;
                double variance = (sumSq / count) - (mean * mean);
                double stdDev = std::sqrt(variance);
                
                // Adaptive contrast enhancement
                double k = 0.5; // Enhancement factor
                double enhanced_value = mean + k * (image[y][x] - mean) * (1.0 + stdDev / 128.0);
                
                enhanced[y][x] = std::max(0.0, std::min(255.0, enhanced_value));
            }
        }
        
        return enhanced;
    }
    
    // Multi-scale edge detection for fine details
    std::vector<std::vector<double>> detectMultiScaleEdges(const std::vector<std::vector<int>>& brightness) {
        int height = brightness.size();
        int width = brightness[0].size();
        std::vector<std::vector<double>> edges(height, std::vector<double>(width, 0));
        
        // Multiple Sobel kernels for different scales
        std::vector<std::vector<std::vector<double>>> kernels = {
            // Fine detail kernel
            {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}},
            // Medium detail kernel
            {{-1, -2, 0, 2, 1}, {-2, -4, 0, 4, 2}, {-3, -6, 0, 6, 3}, {-2, -4, 0, 4, 2}, {-1, -2, 0, 2, 1}},
        };
        
        // Apply each kernel
        for (const auto& kernel : kernels) {
            int ksize = kernel.size();
            int khalf = ksize / 2;
            
            for (int y = khalf; y < height - khalf; y++) {
                for (int x = khalf; x < width - khalf; x++) {
                    double gx = 0, gy = 0;
                    
                    for (int ky = 0; ky < ksize; ky++) {
                        for (int kx = 0; kx < ksize; kx++) {
                            int py = y + ky - khalf;
                            int px = x + kx - khalf;
                            
                            if (py >= 0 && py < height && px >= 0 && px < width) {
                                gx += kernel[ky][kx] * brightness[py][px];
                                gy += kernel[kx][ky] * brightness[py][px];
                            }
                        }
                    }
                    
                    edges[y][x] += std::sqrt(gx * gx + gy * gy) / kernels.size();
                }
            }
        }
        
        return edges;
    }
    
    // Advanced sharpening filter
    std::vector<std::vector<int>> sharpenImage(const std::vector<std::vector<int>>& image) {
        int height = image.size();
        int width = image[0].size();
        std::vector<std::vector<int>> sharpened = image;
        
        // Unsharp mask kernel
        double kernel[3][3] = {
            {-0.5, -1.0, -0.5},
            {-1.0,  7.0, -1.0},
            {-0.5, -1.0, -0.5}
        };
        
        for (int y = 1; y < height - 1; y++) {
            for (int x = 1; x < width - 1; x++) {
                double sum = 0;
                
                for (int ky = -1; ky <= 1; ky++) {
                    for (int kx = -1; kx <= 1; kx++) {
                        sum += kernel[ky + 1][kx + 1] * image[y + ky][x + kx];
                    }
                }
                
                sharpened[y][x] = std::max(0, std::min(255, (int)sum));
            }
        }
        
        return sharpened;
    }
    
    std::vector<std::string> generateUltraASCII(const char* imageData, size_t dataSize) {
        std::vector<std::string> asciiArt(THUMBNAIL_HEIGHT, std::string(THUMBNAIL_WIDTH, ' '));
        
        if (!imageData || dataSize == 0) {
            // Ultra-detailed "no image" display
            std::vector<std::string> noImageArt = {
                "┌────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┐",
                "│                                                                                                                    │",
                "│  ██╗   ██╗ ██████╗ ██╗   ██╗████████╗██╗   ██╗██████╗ ███████╗    ████████╗██╗  ██╗██╗   ██╗███╗   ███╗██████╗  │",
                "│  ╚██╗ ██╔╝██╔═══██╗██║   ██║╚══██╔══╝██║   ██║██╔══██╗██╔════╝    ╚══██╔══╝██║  ██║██║   ██║████╗ ████║██╔══██╗ │",
                "│   ╚████╔╝ ██║   ██║██║   ██║   ██║   ██║   ██║██████╔╝█████╗         ██║   ███████║██║   ██║██╔████╔██║██████╔╝ │",
                "│    ╚██╔╝  ██║   ██║██║   ██║   ██║   ██║   ██║██╔══██╗██╔══╝         ██║   ██╔══██║██║   ██║██║╚██╔╝██║██╔══██╗ │",
                "│     ██║   ╚██████╔╝╚██████╔╝   ██║   ╚██████╔╝██████╔╝███████╗       ██║   ██║  ██║╚██████╔╝██║ ╚═╝ ██║██████╔╝ │",
                "│     ╚═╝    ╚═════╝  ╚═════╝    ╚═╝    ╚═════╝ ╚═════╝ ╚══════╝       ╚═╝   ╚═╝  ╚═╝ ╚═════╝ ╚═╝     ╚═╝╚═════╝  │",
                "│                                                                                                                    │",
                "│                                              ♪ ♫ ♪ ♫ ♪ ♫ ♪ ♫ ♪ ♫ ♪                                              │",
                "│                                                                                                                    │",
                "│                    ╔═══════════════════════════════════════════════════════════════════════╗                      │",
                "│                    ║                                                                       ║                      │",
                "│                    ║  ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░  ║                      │",
                "│                    ║  ░                                                                 ░  ║                      │",
                "│                    ║  ░                    NO THUMBNAIL AVAILABLE                       ░  ║                      │",
                "│                    ║  ░                                                                 ░  ║                      │",
                "│                    ║  ░                    Loading image data...                        ░  ║                      │",
                "│                    ║  ░                                                                 ░  ║                      │",
                "│                    ║  ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░  ║                      │",
                "│                    ║                                                                       ║                      │",
                "│                    ╚═══════════════════════════════════════════════════════════════════════╝                      │",
                "│                                                                                                                    │",
                "│                                              ♪ ♫ ♪ ♫ ♪ ♫ ♪ ♫ ♪ ♫ ♪                                              │",
                "│                                                                                                                    │",
                "└────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┘"
            };
            
            // Ensure proper sizing
            noImageArt.resize(THUMBNAIL_HEIGHT);
            for (int i = 0; i < THUMBNAIL_HEIGHT; i++) {
                if (i < noImageArt.size()) {
                    std::string line = noImageArt[i];
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
        
        // Create pixel matrix with full color information
        std::vector<std::vector<Pixel>> pixels(THUMBNAIL_HEIGHT, std::vector<Pixel>(THUMBNAIL_WIDTH));
        std::vector<std::vector<int>> brightness(THUMBNAIL_HEIGHT, std::vector<int>(THUMBNAIL_WIDTH, 0));
        
                // Process RGB data with advanced sampling
        for (int y = 0; y < THUMBNAIL_HEIGHT; y++) {
            for (int x = 0; x < THUMBNAIL_WIDTH; x++) {
                // Calculate the exact position in the original image
                size_t pixelIndex = (y * THUMBNAIL_WIDTH + x) * 3;
                
                if (pixelIndex + 2 < dataSize) {
                    // Get RGB values
                    pixels[y][x].r = static_cast<unsigned char>(imageData[pixelIndex]);
                    pixels[y][x].g = static_cast<unsigned char>(imageData[pixelIndex + 1]);
                    pixels[y][x].b = static_cast<unsigned char>(imageData[pixelIndex + 2]);
                    
                    // Apply gamma correction for more accurate brightness
                    int correctedR = gammaCorrect(pixels[y][x].r);
                    int correctedG = gammaCorrect(pixels[y][x].g);
                    int correctedB = gammaCorrect(pixels[y][x].b);
                    
                    // Calculate perceived brightness using advanced formula
                    pixels[y][x].brightness = (correctedR * 299 + correctedG * 587 + correctedB * 114) / 1000;
                    brightness[y][x] = pixels[y][x].brightness;
                }
            }
        }
        
        // Apply advanced image processing pipeline
        brightness = sharpenImage(brightness);
        brightness = equalizeHistogram(brightness);
        auto enhancedContrast = enhanceLocalContrast(brightness);
        auto edges = detectMultiScaleEdges(brightness);
        
        // Calculate local texture and detail metrics
        std::vector<std::vector<double>> texture(THUMBNAIL_HEIGHT, std::vector<double>(THUMBNAIL_WIDTH, 0));
        for (int y = 1; y < THUMBNAIL_HEIGHT - 1; y++) {
            for (int x = 1; x < THUMBNAIL_WIDTH - 1; x++) {
                // Calculate local variance as texture metric
                double sum = 0, sumSq = 0;
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        double val = enhancedContrast[y + dy][x + dx];
                        sum += val;
                        sumSq += val * val;
                    }
                }
                double mean = sum / 9.0;
                double variance = (sumSq / 9.0) - (mean * mean);
                texture[y][x] = std::sqrt(variance);
            }
        }
        
        // Final ASCII conversion with ultra-detailed character mapping
        for (int y = 0; y < THUMBNAIL_HEIGHT; y++) {
            for (int x = 0; x < THUMBNAIL_WIDTH; x++) {
                // Combine multiple metrics for character selection
                double brightness_factor = enhancedContrast[y][x] / 255.0;
                double edge_factor = std::min(1.0, edges[y][x] / 128.0);
                double texture_factor = std::min(1.0, texture[y][x] / 64.0);
                
                // Weighted combination
                double combined = brightness_factor * 0.6 + edge_factor * 0.2 + texture_factor * 0.2;
                
                // Special handling for very dark and very bright areas
                if (brightness_factor < 0.05) {
                    asciiArt[y][x] = ' ';  // Pure black
                } else if (brightness_factor > 0.95 && edge_factor < 0.1) {
                    asciiArt[y][x] = '@';  // Pure white
                } else {
                    // Map to ultra-detailed character set
                    int charIndex = static_cast<int>(combined * (ASCII_CHARS_ULTRA.length() - 1));
                    charIndex = std::max(0, std::min((int)ASCII_CHARS_ULTRA.length() - 1, charIndex));
                    asciiArt[y][x] = ASCII_CHARS_ULTRA[charIndex];
                }
                
                // Edge enhancement - use specific characters for strong edges
                if (edge_factor > 0.7) {
                    if (std::abs(enhancedContrast[y][x] - enhancedContrast[y][x-1]) > 
                        std::abs(enhancedContrast[y][x] - enhancedContrast[y-1][x])) {
                        asciiArt[y][x] = '|';  // Vertical edge
                    } else {
                        asciiArt[y][x] = '-';  // Horizontal edge
                    }
                }
            }
        }
        
        // Post-processing: smooth out noise in uniform areas
        std::vector<std::string> smoothed = asciiArt;
        for (int y = 1; y < THUMBNAIL_HEIGHT - 1; y++) {
            for (int x = 1; x < THUMBNAIL_WIDTH - 1; x++) {
                // Check if surrounding area is uniform
                std::map<char, int> charCount;
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        charCount[asciiArt[y + dy][x + dx]]++;
                    }
                }
                
                // If one character dominates, use it
                for (const auto& pair : charCount) {
                    if (pair.second >= 6) {
                        smoothed[y][x] = pair.first;
                        break;
                    }
                }
            }
        }
        
        return smoothed;
    }
    
    std::vector<std::string> generateASCIIArt(const char* imageData, size_t dataSize) {
        return generateUltraASCII(imageData, dataSize);
    }
    
    std::vector<std::string> extractAlbumArtASCII(const std::string& filePath) {
        TagLib::MPEG::File file(filePath.c_str());
        
        if (!file.isValid()) {
            return generateUltraASCII(nullptr, 0);
        }
        
        TagLib::ID3v2::Tag* id3v2Tag = file.ID3v2Tag();
        if (!id3v2Tag) {
            return generateUltraASCII(nullptr, 0);
        }
        
        TagLib::ID3v2::FrameList frameList = id3v2Tag->frameList("APIC");
        if (frameList.isEmpty()) {
            return generateUltraASCII(nullptr, 0);
        }
        
        TagLib::ID3v2::AttachedPictureFrame* pictureFrame = 
            static_cast<TagLib::ID3v2::AttachedPictureFrame*>(frameList.front());
        
        if (!pictureFrame) {
            return generateUltraASCII(nullptr, 0);
        }
        
        TagLib::ByteVector imageData = pictureFrame->picture();
        return generateUltraASCII(imageData.data(), imageData.size());
    }
    
    std::vector<std::string> getYouTubeThumbnailASCII(const std::string& video_id) {
        std::string temp_dir = "/tmp/uwu_thumbnails/";
        std::string temp_jpg = temp_dir + video_id + ".jpg";
        std::string temp_raw = temp_dir + video_id + ".rgb";
        
        // Create temp directory if it doesn't exist
        std::filesystem::create_directories(temp_dir);
        
        // Try different thumbnail qualities (prefer highest)
        std::vector<std::string> thumbnail_urls = {
            "https://img.youtube.com/vi/" + video_id + "/maxresdefault.jpg",
            "https://img.youtube.com/vi/" + video_id + "/sddefault.jpg",
            "https://img.youtube.com/vi/" + video_id + "/hqdefault.jpg"
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
            return generateUltraASCII(nullptr, 0);
        }
        
        // Convert JPEG to raw RGB data with maximum quality
        std::string convert_cmd = "magick convert \"" + temp_jpg + 
                                  "\" -colorspace sRGB -resize " + 
                                  std::to_string(THUMBNAIL_WIDTH) + "x" + 
                                  std::to_string(THUMBNAIL_HEIGHT) + 
                                  "! -depth 8 rgb:\"" + temp_raw + "\" 2>/dev/null";
        
        if (system(convert_cmd.c_str()) != 0) {
            std::filesystem::remove(temp_jpg);
            return generateUltraASCII(nullptr, 0);
        }
        
        // Read the raw RGB data
        std::ifstream file(temp_raw, std::ios::binary);
        if (!file) {
            std::filesystem::remove(temp_jpg);
            std::filesystem::remove(temp_raw);
            return generateUltraASCII(nullptr, 0);
        }
        
        file.seekg(0, std::ios::end);
        size_t file_size = file.tellg();
        file.seekg(0, std::ios::beg);
        
        std::vector<char> image_data(file_size);
        file.read(image_data.data(), file_size);
        file.close();
        
        // Generate ultra-realistic ASCII art
        std::vector<std::string> ascii_art = generateUltraASCII(image_data.data(), file_size);
        
        // Clean up temp files
        std::filesystem::remove(temp_jpg);
        std::filesystem::remove(temp_raw);
        
        return ascii_art;
    }
}

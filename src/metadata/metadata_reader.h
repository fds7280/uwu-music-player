#ifndef METADATA_READER_H
#define METADATA_READER_H

#include <string>

namespace Metadata {
    struct TrackInfo {
        std::string title;
        std::string artist;
        std::string album;
        int year;
        int duration; // in seconds
        
        // Default constructor
        TrackInfo() : title("Unknown Title"), artist("Unknown Artist"), 
                     album("Unknown Album"), year(0), duration(0) {}
    };
    
    // Function to read metadata from an audio file
    TrackInfo readMetadata(const std::string& filePath);
}

#endif // METADATA_READER_H

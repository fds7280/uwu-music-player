#include "metadata/metadata_reader.h"
#include <taglib/fileref.h>
#include <taglib/tag.h>

namespace Metadata {
    TrackInfo readMetadata(const std::string& filePath) {
        TrackInfo info;
        
        // Initialize with default values
        info.title = "Unknown Title";
        info.artist = "Unknown Artist";
        info.album = "Unknown Album";
        info.year = 0;
        info.duration = 0;
        
        TagLib::FileRef f(filePath.c_str());
        
        if (!f.isNull() && f.tag()) {
            TagLib::Tag *tag = f.tag();
            
            // Only update if the tag has actual content
            if (!tag->title().isEmpty()) {
                info.title = tag->title().toCString(true);
            }
            if (!tag->artist().isEmpty()) {
                info.artist = tag->artist().toCString(true);
            }
            if (!tag->album().isEmpty()) {
                info.album = tag->album().toCString(true);
            }
            if (tag->year() > 0) {
                info.year = tag->year();
            }
        }
        
        if (!f.isNull() && f.audioProperties()) {
            info.duration = f.audioProperties()->length();
        }
        
        return info;
    }
}

#ifndef YOUTUBE_STREAM_H
#define YOUTUBE_STREAM_H

#include <string>
#include <vector>

namespace Streaming {
    struct SearchResult {
        std::string id;
        std::string title;
    };
    
    // YouTube operations
    std::vector<SearchResult> searchYouTube(const std::string& query);
    void progressiveStreamYouTube(const std::string& video_id, 
                                  const std::string& title, 
                                  const std::string& cache_dir);
    
    // Cache management
    bool isCached(const std::string& video_id, const std::string& cache_dir);
    std::string getCachedFilePath(const std::string& video_id, const std::string& cache_dir);
 
    //Cache cleanup function
    void cleanupCache(const std::string& cache_dir, const std::string& current_video_id);


}

#endif // YOUTUBE_STREAM_H

#ifndef SEARCH_UI_H
#define SEARCH_UI_H

#include <string>
#include <vector>
#include "../streaming/youtube_stream.h"

namespace UI {
    std::string getSearchQuery(int max_y, int max_x);
    Streaming::SearchResult selectFromResults(const std::vector<Streaming::SearchResult>& results);
    void runOnlineMode();
}

#endif // SEARCH_UI_H

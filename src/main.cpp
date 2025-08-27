#include "ui/ui_common.h"
#include "ui/file_browser.h"
#include "ui/playback_ui.h"
#include "ui/search_ui.h"
#include "audio/audio_player.h"
#include "utils/utils.h"

int main() {
    UI::init();
    
    UI::Mode mode = UI::promptModeSelection();
    
    clear();
    refresh();

    if (mode == UI::OFFLINE_MODE) {
        std::string music_dir = UI::runFileBrowser("/home");
        if (!music_dir.empty()) {
            UI::runPlaybackTUI(music_dir);
        }
    } else if (mode == UI::ONLINE_MODE) {
        UI::runOnlineMode();
    } else if (mode == UI::PLAYLIST_MODE) {
        UI::runPlaylistMode();
    }
    
    UI::cleanup();
    
    // Final cleanup of audio resources
    if (Audio::is_playing) {
        Audio::StopAudio();
    }

    return 0;
}

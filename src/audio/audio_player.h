#ifndef AUDIO_PLAYER_H
#define AUDIO_PLAYER_H

#include <string>
#include <atomic>

namespace Audio {
    // Global playback state
    extern std::atomic<bool> is_playing;
    extern std::atomic<bool> is_paused;
    extern std::atomic<long> total_frames;
    extern std::atomic<long> current_frame;
    
    // Playback control functions
    void PlayAudio(const std::string& file_path);
    void StopAudio();
    void PauseAudio();
    void ResumeAudio();
    
    // Get current playback info
    float GetProgress();
    int GetCurrentTimeSeconds();
    int GetTotalTimeSeconds();
    int GetSampleRate();
}

#endif // AUDIO_PLAYER_H

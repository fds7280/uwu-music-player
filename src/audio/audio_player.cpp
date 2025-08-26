#include "audio/audio_player.h"
#include <pipewire/pipewire.h>
#include <spa/param/audio/format-utils.h>
#include <sndfile.h>
#include <thread>
#include <cstring>

namespace Audio {
    std::atomic<bool> is_playing(false);
    std::atomic<bool> is_paused(false);
    std::atomic<long> total_frames(0);
    std::atomic<long> current_frame(0);
    
    // PipeWire data structure
    struct PWData {
        struct pw_main_loop *loop;
        struct pw_stream *stream;
        struct spa_hook stream_listener;
        
        SNDFILE* sf;
        SF_INFO sfinfo;
        
        std::thread loop_thread;
        std::atomic<bool> should_stop{false};
    };
    
    // Global PipeWire data pointer
    static PWData* g_pw_data = nullptr;
    
    // PipeWire stream process callback
    static void on_process(void *userdata) {
        PWData *data = static_cast<PWData*>(userdata);
        struct pw_buffer *b;
        struct spa_buffer *buf;
        float *dst;
        uint32_t n_frames;
        
        if ((b = pw_stream_dequeue_buffer(data->stream)) == NULL) {
            pw_log_warn("out of buffers: %m");
            return;
        }
        
        buf = b->buffer;
        if ((dst = (float*)buf->datas[0].data) == NULL)
            return;
        
        n_frames = buf->datas[0].maxsize / sizeof(float) / data->sfinfo.channels;
        
        if (is_paused) {
            memset(dst, 0, n_frames * sizeof(float) * data->sfinfo.channels);
        } else {
            sf_count_t frames_read = sf_readf_float(data->sf, dst, n_frames);
            current_frame = sf_seek(data->sf, 0, SF_SEEK_CUR);
            
            if (frames_read < n_frames) {
                memset(dst + frames_read * data->sfinfo.channels, 0, 
                       (n_frames - frames_read) * sizeof(float) * data->sfinfo.channels);
                if (frames_read == 0) {
                    is_playing = false;
                }
            }
        }
        
        buf->datas[0].chunk->offset = 0;
        buf->datas[0].chunk->stride = sizeof(float) * data->sfinfo.channels;
        buf->datas[0].chunk->size = n_frames * sizeof(float) * data->sfinfo.channels;
        
        pw_stream_queue_buffer(data->stream, b);
    }
    
    static const struct pw_stream_events stream_events = {
        PW_VERSION_STREAM_EVENTS,
        .process = on_process,
    };
    
    void PlayAudio(const std::string& file_path) {
        if (g_pw_data) {
            StopAudio();
        }
        
        g_pw_data = new PWData();
        memset(&g_pw_data->sfinfo, 0, sizeof(g_pw_data->sfinfo));
        
        g_pw_data->sf = sf_open(file_path.c_str(), SFM_READ, &g_pw_data->sfinfo);
        if (!g_pw_data->sf) {
            delete g_pw_data;
            g_pw_data = nullptr;
            return;
        }
        
        sf_seek(g_pw_data->sf, 0, SF_SEEK_END);
        total_frames = sf_seek(g_pw_data->sf, 0, SF_SEEK_CUR);
        sf_seek(g_pw_data->sf, 0, SF_SEEK_SET);
        current_frame = 0;
        
        pw_init(nullptr, nullptr);
        
        g_pw_data->loop = pw_main_loop_new(nullptr);
        if (!g_pw_data->loop) {
            sf_close(g_pw_data->sf);
            delete g_pw_data;
            g_pw_data = nullptr;
            return;
        }
        
        g_pw_data->stream = pw_stream_new_simple(
            pw_main_loop_get_loop(g_pw_data->loop),
            "UwU Music Player",
            pw_properties_new(
                PW_KEY_MEDIA_TYPE, "Audio",
                PW_KEY_MEDIA_CATEGORY, "Playback",
                PW_KEY_MEDIA_ROLE, "Music",
                nullptr),
            &stream_events,
            g_pw_data);
        
        if (!g_pw_data->stream) {
            pw_main_loop_destroy(g_pw_data->loop);
            sf_close(g_pw_data->sf);
            delete g_pw_data;
            g_pw_data = nullptr;
            return;
        }
        
        uint8_t buffer[1024];
        struct spa_pod_builder b = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
        const struct spa_pod *params[1];
        
        struct spa_audio_info_raw spa_audio_info = {};
        spa_audio_info.format = SPA_AUDIO_FORMAT_F32;
        spa_audio_info.rate = g_pw_data->sfinfo.samplerate;
        spa_audio_info.channels = g_pw_data->sfinfo.channels;
        
        if (g_pw_data->sfinfo.channels == 1) {
            spa_audio_info.position[0] = SPA_AUDIO_CHANNEL_MONO;
        } else if (g_pw_data->sfinfo.channels == 2) {
            spa_audio_info.position[0] = SPA_AUDIO_CHANNEL_FL;
            spa_audio_info.position[1] = SPA_AUDIO_CHANNEL_FR;
        }
        
        params[0] = spa_format_audio_raw_build(&b, SPA_PARAM_EnumFormat, &spa_audio_info);
        
        pw_stream_connect(g_pw_data->stream,
                          PW_DIRECTION_OUTPUT,
                          PW_ID_ANY,
                          static_cast<pw_stream_flags>(PW_STREAM_FLAG_AUTOCONNECT |
                                                       PW_STREAM_FLAG_MAP_BUFFERS |
                                                       PW_STREAM_FLAG_RT_PROCESS),
                          params, 1);
        
        g_pw_data->loop_thread = std::thread([&]() {
            pw_main_loop_run(g_pw_data->loop);
        });
    }
    
    void StopAudio() {
        if (!g_pw_data) return;

        is_playing = false;
        is_paused = false;

        g_pw_data->should_stop = true;
        if (g_pw_data->loop) {
            pw_main_loop_quit(g_pw_data->loop);
        }
        if (g_pw_data->loop_thread.joinable()) {
            g_pw_data->loop_thread.join();
        }
        if (g_pw_data->stream) pw_stream_destroy(g_pw_data->stream);
        if (g_pw_data->loop) pw_main_loop_destroy(g_pw_data->loop);
        if (g_pw_data->sf) sf_close(g_pw_data->sf);
        
        delete g_pw_data;
        g_pw_data = nullptr;
        
        total_frames = 0;
        current_frame = 0;
    }
    
    void PauseAudio() {
        is_paused = true;
    }
    
    void ResumeAudio() {
        is_paused = false;
    }
    
    float GetProgress() {
        if (total_frames == 0) return 0.0f;
        return static_cast<float>(current_frame) / static_cast<float>(total_frames);
    }
    
    int GetSampleRate() {
        if (!g_pw_data) return 0;
        return g_pw_data->sfinfo.samplerate;
    }
    
    int GetCurrentTimeSeconds() {
        int sampleRate = GetSampleRate();
        if (sampleRate == 0) return 0;
        return static_cast<int>(current_frame / sampleRate);
    }
    
    int GetTotalTimeSeconds() {
        int sampleRate = GetSampleRate();
        if (sampleRate == 0) return 0;
        return static_cast<int>(total_frames / sampleRate);
    }
}

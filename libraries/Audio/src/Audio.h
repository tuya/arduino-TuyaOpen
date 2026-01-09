#ifndef __AUDIO_H__
#define __AUDIO_H__

extern "C" {
#include "tuya_cloud_types.h"
#include "tal_api.h"
#include "tuya_ringbuf.h"
#include "tkl_output.h"
#include "tkl_memory.h"
#include "board_com_api.h"
#include "tdl_audio_manage.h"
}

class Audio {
    public:
    /**
     * @brief Audio status enumeration
     */
    typedef enum {
        AUDIO_STATUS_IDLE = 0,        // Idle, ready for operation
        AUDIO_STATUS_RECORD_START,    // Recording started
        AUDIO_STATUS_RECORDING,       // Recording in progress
        AUDIO_STATUS_RECORD_END,      // Recording ended
        AUDIO_STATUS_PLAY_START,      // Playback started
        AUDIO_STATUS_PLAYING,         // Playback in progress
        AUDIO_STATUS_PLAY_END,        // Playback ended
    } AUDIO_STATUS_E;

    typedef struct {
        uint32_t duration_ms;    // Maximum recordable duration in ms
        uint8_t volume;          // Volume (0-100)
        uint32_t sample_rate;    // Sample rate
        uint8_t pin;             // Pin number
    } AUDIO_CONFIG_T;

    /**
     * @brief Audio frame callback function type
     * @param data Pointer to audio frame data (PCM 16-bit)
     * @param len Length of audio data in bytes
     */
    typedef void (*AudioFrameCallback)(uint8_t* data, uint32_t len);

    Audio();
    ~Audio();

    // Initialization and control
    OPERATE_RET begin(const AUDIO_CONFIG_T* config = NULL);
    void end();

    // Recording control
    OPERATE_RET startRecord();
    void stopRecord();
    
    // Playback control - play data directly
    OPERATE_RET play(uint8_t* data, uint32_t len);
    OPERATE_RET playRecordedData();
    void stopPlay();
    bool shouldStopPlay() { return stop_play_flag; }
    void resetStopFlag() { stop_play_flag = false; }

    // Volume control
    OPERATE_RET setVolume(uint8_t volume);
    uint8_t getVolume();

    // Status interface
    AUDIO_STATUS_E getStatus();
    bool isIdle();
    bool isRecording();
    bool isPlaying();

    // Recorded data access
    uint32_t getRecordedDataLen();
    uint32_t readRecordedData(uint8_t* buffer, uint32_t len);
    void clearRecordedData();

    // Audio frame callback for real-time processing
    void setAudioFrameCallback(AudioFrameCallback cb);

    // Internal data access (advanced)
    TUYA_RINGBUFF_T getRecordBuffer() { return sg_recorder_pcm_rb; }
    TDL_AUDIO_INFO_T* getAudioInfo() { return &sg_audio_info; }
    TDL_AUDIO_HANDLE_T getAudioHandle() { return sg_audio_hdl; }
    AudioFrameCallback getFrameCallback() { return frame_callback; }

    private:
    AUDIO_STATUS_E sg_audio_status;
    TDL_AUDIO_HANDLE_T sg_audio_hdl;
    TDL_AUDIO_INFO_T sg_audio_info;
    TUYA_RINGBUFF_T sg_recorder_pcm_rb;
    uint32_t record_duration_ms;
    uint8_t current_volume;
    AudioFrameCallback frame_callback;
    bool stop_play_flag;

    void setStatus(AUDIO_STATUS_E status);

    // Private helper functions
    static void audio_frame_callback_wrapper(TDL_AUDIO_FRAME_FORMAT_E type, 
                                            TDL_AUDIO_STATUS_E status,
                                            uint8_t* data, 
                                            uint32_t len);
};

#endif // __AUDIO_H__
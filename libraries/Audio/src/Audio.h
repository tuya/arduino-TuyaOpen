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
    typedef enum {
        RECORDER_STATUS_IDLE = 0,
        RECORDER_STATUS_START,
        RECORDER_STATUS_RECORDING,
        RECORDER_STATUS_END,
        RECORDER_STATUS_PLAYING,
    } RECORDER_STATUS_E;

    typedef struct {
        uint32_t duration_ms;    // Maximum recordable duration in ms
        uint8_t volume;          // Volume (0-100)
        uint32_t sample_rate;    // Sample rate
    } AUDIO_CONFIG_T;

    Audio();
    ~Audio();

    // Initialization and control
    OPERATE_RET begin(const AUDIO_CONFIG_T* config = NULL);
    void end();

    // Recording control
    OPERATE_RET startRecord();
    void stopRecord();
    OPERATE_RET playback();
    void stopPlayback();

    // Volume control
    OPERATE_RET setVolume(uint8_t volume);
    uint8_t getVolume();

    // Status and data access
    RECORDER_STATUS_E getStatus();
    void setStatus(RECORDER_STATUS_E status);
    uint32_t getRecordedDataLen();
    OPERATE_RET readRecordedData(uint8_t* buffer, uint32_t len, uint32_t* out_len);
    void clearRecordedData();

    // Audio frame callback
    typedef void (*AudioFrameCallback)(uint8_t* data, uint32_t len);
    void setAudioFrameCallback(AudioFrameCallback cb);

    // Data access
    TUYA_RINGBUFF_T getRecordBuffer() { return sg_recorder_pcm_rb; }
    TDL_AUDIO_INFO_T* getAudioInfo() { return &sg_audio_info; }
    TDL_AUDIO_HANDLE_T getAudioHandle() { return sg_audio_hdl; }

    private:
    RECORDER_STATUS_E sg_recorder_status;
    TDL_AUDIO_HANDLE_T sg_audio_hdl;
    TDL_AUDIO_INFO_T sg_audio_info;
    TUYA_RINGBUFF_T sg_recorder_pcm_rb;
    uint32_t record_duration_ms;
    uint8_t current_volume;
    AudioFrameCallback frame_callback;

    // Private helper functions
    static void audio_frame_callback_wrapper(TDL_AUDIO_FRAME_FORMAT_E type, 
                                            TDL_AUDIO_STATUS_E status,
                                            uint8_t* data, 
                                            uint32_t len);
};

#endif // __AUDIO_H__
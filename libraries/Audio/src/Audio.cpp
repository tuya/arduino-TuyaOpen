#include "Audio.h"
#include "tdd_audio.h"
// Global instance pointer for C callback
static Audio* g_audio_instance = NULL;
#define BOARD_SPEAKER_EN_PIN         TUYA_GPIO_NUM_28

// C callback wrapper function
static void audio_frame_callback_internal(TDL_AUDIO_FRAME_FORMAT_E type, 
                                         TDL_AUDIO_STATUS_E status,
                                         uint8_t* data, 
                                         uint32_t len)
{
    if (g_audio_instance != NULL) {
        if (g_audio_instance->getRecordBuffer() != NULL) {
            tuya_ring_buff_write(g_audio_instance->getRecordBuffer(), data, len);
        }
    }
}

// Constructor
Audio::Audio() 
    : sg_recorder_status(RECORDER_STATUS_IDLE), 
      sg_audio_hdl(NULL), 
      sg_recorder_pcm_rb(NULL),
      record_duration_ms(3000),  // Default 3 seconds
      current_volume(60),
      frame_callback(NULL)
{
    memset(&sg_audio_info, 0, sizeof(TDL_AUDIO_INFO_T));
    g_audio_instance = this;
}

// Destructor
Audio::~Audio()
{
    end();
    if (g_audio_instance == this) {
        g_audio_instance = NULL;
    }
}

// Initialization
OPERATE_RET Audio::begin(const AUDIO_CONFIG_T* config)
{
    OPERATE_RET rt = OPRT_OK;
    uint32_t buf_len = 0;

    // Use provided config or defaults
    if (config != NULL) {
        record_duration_ms = config->duration_ms;
        current_volume = config->volume;
    }

#if defined(AUDIO_CODEC_NAME)
    TDD_AUDIO_T5AI_T cfg = {0};
    memset(&cfg, 0, sizeof(TDD_AUDIO_T5AI_T));

#if defined(ENABLE_AUDIO_AEC) && (ENABLE_AUDIO_AEC == 1)
    cfg.aec_enable = 1;
#else
    cfg.aec_enable = 0;
#endif

    cfg.ai_chn = TKL_AI_0;
    cfg.sample_rate = TKL_AUDIO_SAMPLE_16K;
    cfg.data_bits = TKL_AUDIO_DATABITS_16;
    cfg.channel = TKL_AUDIO_CHANNEL_MONO;

    cfg.spk_sample_rate = TKL_AUDIO_SAMPLE_16K;
    cfg.spk_pin = BOARD_SPEAKER_EN_PIN;
    cfg.spk_pin_polarity = TUYA_GPIO_LEVEL_LOW;

    TUYA_CALL_ERR_RETURN(tdd_audio_register(AUDIO_CODEC_NAME, cfg));
#endif

    // Find and open audio device
    static const char audio_name[] = "audio_codec";
    rt = tdl_audio_find((char*)audio_name, &sg_audio_hdl);
    if (rt != OPRT_OK) {
        PR_ERR("Failed to find audio device");
        return rt;
    }

    rt = tdl_audio_open(sg_audio_hdl, audio_frame_callback_internal);
    if (rt != OPRT_OK) {
        PR_ERR("Failed to open audio device");
        return rt;
    }

    rt = tdl_audio_get_info(sg_audio_hdl, &sg_audio_info);
    if (rt != OPRT_OK) {
        PR_ERR("Failed to get audio info");
        return rt;
    }

    if (0 == sg_audio_info.frame_size || 0 == sg_audio_info.sample_tm_ms) {
        PR_ERR("Invalid audio info");
        tdl_audio_close(sg_audio_hdl);
        return OPRT_INVALID_PARM;
    }

    // Create ring buffer for recording
    buf_len = (record_duration_ms / sg_audio_info.sample_tm_ms) * sg_audio_info.frame_size;
    rt = tuya_ring_buff_create(buf_len, (RINGBUFF_TYPE_E)OVERFLOW_PSRAM_STOP_TYPE, &sg_recorder_pcm_rb);
    if (rt != OPRT_OK) {
        PR_ERR("Failed to create ring buffer");
        tdl_audio_close(sg_audio_hdl);
        return rt;
    }

    // Set volume
    setVolume(current_volume);

    sg_recorder_status = RECORDER_STATUS_IDLE;
    PR_NOTICE("Audio initialized successfully");

    return OPRT_OK;
}

// Cleanup
void Audio::end()
{
    if (sg_audio_hdl != NULL) {
        tdl_audio_close(sg_audio_hdl);
        sg_audio_hdl = NULL;
    }

    if (sg_recorder_pcm_rb != NULL) {
        tuya_ring_buff_free(sg_recorder_pcm_rb);
        sg_recorder_pcm_rb = NULL;
    }

    sg_recorder_status = RECORDER_STATUS_IDLE;
    PR_NOTICE("Audio closed");
}

// Start recording
OPERATE_RET Audio::startRecord()
{
    if (sg_audio_hdl == NULL || sg_recorder_pcm_rb == NULL) {
        PR_ERR("Audio not initialized");
        return OPRT_NOT_FOUND;
    }

    tuya_ring_buff_reset(sg_recorder_pcm_rb);
    sg_recorder_status = RECORDER_STATUS_RECORDING;
    PR_NOTICE("Recording started");

    return OPRT_OK;
}

// Stop recording
void Audio::stopRecord()
{
    if (sg_recorder_status == RECORDER_STATUS_RECORDING) {
        sg_recorder_status = RECORDER_STATUS_END;
        PR_NOTICE("Recording stopped");
    }
}

// Playback recorded audio
OPERATE_RET Audio::playback()
{
    if (sg_recorder_pcm_rb == NULL || sg_audio_hdl == NULL || 
        sg_audio_info.frame_size == 0) {
        PR_ERR("Audio not properly initialized");
        return OPRT_NOT_FOUND;
    }

    uint32_t data_len = tuya_ring_buff_used_size_get(sg_recorder_pcm_rb);
    if (data_len == 0) {
        PR_NOTICE("No data in recorder buffer");
        return OPRT_OK;
    }

    uint32_t out_len = 0;
    uint8_t* frame_buf = (uint8_t*)tkl_system_psram_malloc(sg_audio_info.frame_size);
    if (frame_buf == NULL) {
        PR_ERR("Memory allocation failed");
        return OPRT_MALLOC_FAILED;
    }

    sg_recorder_status = RECORDER_STATUS_PLAYING;
    PR_NOTICE("Playback started");

    while (true) {
        memset(frame_buf, 0, sg_audio_info.frame_size);
        out_len = 0;

        data_len = tuya_ring_buff_used_size_get(sg_recorder_pcm_rb);
        if (data_len == 0) {
            PR_NOTICE("Playback completed");
            break;
        }

        if (data_len > sg_audio_info.frame_size) {
            tuya_ring_buff_read(sg_recorder_pcm_rb, frame_buf, sg_audio_info.frame_size);
            out_len = sg_audio_info.frame_size;
        } else {
            tuya_ring_buff_read(sg_recorder_pcm_rb, frame_buf, data_len);
            out_len = data_len;
        }

        tdl_audio_play(sg_audio_hdl, frame_buf, out_len);
    }

    if (frame_buf != NULL) {
        tkl_system_psram_free(frame_buf);
        frame_buf = NULL;
    }

    sg_recorder_status = RECORDER_STATUS_IDLE;
    return OPRT_OK;
}

// Stop playback
void Audio::stopPlayback()
{
    if (sg_recorder_status == RECORDER_STATUS_PLAYING) {
        sg_recorder_status = RECORDER_STATUS_IDLE;
        PR_NOTICE("Playback stopped");
    }
}

// Set volume
OPERATE_RET Audio::setVolume(uint8_t volume)
{
    if (sg_audio_hdl == NULL) {
        return OPRT_NOT_FOUND;
    }

    if (volume > 100) {
        volume = 100;
    }

    current_volume = volume;
    OPERATE_RET rt = tdl_audio_volume_set(sg_audio_hdl, volume);
    if (rt == OPRT_OK) {
        PR_NOTICE("Volume set to %d", volume);
    }
    return rt;
}

// Get volume
uint8_t Audio::getVolume()
{
    return current_volume;
}

// Get recorder status
Audio::RECORDER_STATUS_E Audio::getStatus()
{
    return sg_recorder_status;
}

// Set recorder status
void Audio::setStatus(RECORDER_STATUS_E status)
{
    sg_recorder_status = status;
}

// Get recorded data length
uint32_t Audio::getRecordedDataLen()
{
    if (sg_recorder_pcm_rb == NULL) {
        return 0;
    }
    return tuya_ring_buff_used_size_get(sg_recorder_pcm_rb);
}

// Read recorded data
OPERATE_RET Audio::readRecordedData(uint8_t* buffer, uint32_t len, uint32_t* out_len)
{
    if (sg_recorder_pcm_rb == NULL || buffer == NULL || out_len == NULL) {
        return OPRT_INVALID_PARM;
    }

    uint32_t available = tuya_ring_buff_used_size_get(sg_recorder_pcm_rb);
    *out_len = (len < available) ? len : available;

    if (*out_len > 0) {
        tuya_ring_buff_read(sg_recorder_pcm_rb, buffer, *out_len);
    }

    return OPRT_OK;
}

// Clear recorded data
void Audio::clearRecordedData()
{
    if (sg_recorder_pcm_rb != NULL) {
        tuya_ring_buff_reset(sg_recorder_pcm_rb);
        PR_NOTICE("Recorded data cleared");
    }
}

// Set audio frame callback
void Audio::setAudioFrameCallback(AudioFrameCallback cb)
{
    frame_callback = cb;
}

// Static callback wrapper
void Audio::audio_frame_callback_wrapper(TDL_AUDIO_FRAME_FORMAT_E type, 
                                        TDL_AUDIO_STATUS_E status,
                                        uint8_t* data, 
                                        uint32_t len)
{
    if (g_audio_instance != NULL && g_audio_instance->frame_callback != NULL) {
        g_audio_instance->frame_callback(data, len);
    }
}
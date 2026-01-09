#include "Audio.h"
#include "tdd_audio.h"
// Global instance pointer for C callback
static Audio* g_audio_instance = NULL;

// C callback wrapper function - writes to ring buffer and calls user callback
static void audio_frame_callback_internal(TDL_AUDIO_FRAME_FORMAT_E type, 
                                         TDL_AUDIO_STATUS_E status,
                                         uint8_t* data, 
                                         uint32_t len)
{
    if (g_audio_instance != NULL) {
        // Write to ring buffer if recording
        if (g_audio_instance->isRecording() && g_audio_instance->getRecordBuffer() != NULL) {
            tuya_ring_buff_write(g_audio_instance->getRecordBuffer(), data, len);
        }
        
        // Call user callback if set
        if (g_audio_instance->getFrameCallback() != NULL) {
            g_audio_instance->getFrameCallback()(data, len);
        }
    }
}

// Constructor
Audio::Audio() 
    : sg_audio_status(AUDIO_STATUS_IDLE), 
      sg_audio_hdl(NULL), 
      sg_recorder_pcm_rb(NULL),
      record_duration_ms(3000),  // Default 3 seconds
      current_volume(60),
      frame_callback(NULL),
      stop_play_flag(false)
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
    cfg.spk_pin = config->pin;
    cfg.spk_pin_polarity = TUYA_GPIO_LEVEL_LOW;

    TUYA_CALL_ERR_RETURN(tdd_audio_register(AUDIO_CODEC_NAME, cfg));
#endif

    // Find and open audio device
    // static const char audio_name[] = "audio_codec";
    rt = tdl_audio_find((char*)AUDIO_CODEC_NAME, &sg_audio_hdl);
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

    sg_audio_status = AUDIO_STATUS_IDLE;
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

    sg_audio_status = AUDIO_STATUS_IDLE;
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
    setStatus(AUDIO_STATUS_RECORD_START);
    setStatus(AUDIO_STATUS_RECORDING);
    PR_NOTICE("Recording started");

    return OPRT_OK;
}

// Stop recording
void Audio::stopRecord()
{
    if (sg_audio_status == AUDIO_STATUS_RECORDING) {
        setStatus(AUDIO_STATUS_RECORD_END);
        PR_NOTICE("Recording stopped");
    }
}

// Play data directly (streaming playback)
OPERATE_RET Audio::play(uint8_t* data, uint32_t len)
{
    if (sg_audio_hdl == NULL || data == NULL || len == 0) {
        PR_ERR("Invalid parameters for play");
        return OPRT_INVALID_PARM;
    }

    if (sg_audio_status != AUDIO_STATUS_PLAYING) {
        setStatus(AUDIO_STATUS_PLAY_START);
        setStatus(AUDIO_STATUS_PLAYING);
    }

    OPERATE_RET rt = tdl_audio_play(sg_audio_hdl, data, len);
    return rt;
}

// Playback recorded audio from ring buffer
OPERATE_RET Audio::playRecordedData()
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

    setStatus(AUDIO_STATUS_PLAY_START);
    setStatus(AUDIO_STATUS_PLAYING);
    stop_play_flag = false;
    PR_NOTICE("Playback started");

    while (!stop_play_flag) {
        memset(frame_buf, 0, sg_audio_info.frame_size);
        out_len = 0;

        data_len = tuya_ring_buff_used_size_get(sg_recorder_pcm_rb);
        if (data_len == 0) {
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

    setStatus(AUDIO_STATUS_PLAY_END);
    setStatus(AUDIO_STATUS_IDLE);
    PR_NOTICE("Playback completed");
    
    return OPRT_OK;
}

// Stop playback
void Audio::stopPlay()
{
    stop_play_flag = true;
    setStatus(AUDIO_STATUS_PLAY_END);
    setStatus(AUDIO_STATUS_IDLE);
    PR_NOTICE("Playback stopped");
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

// Get audio status
Audio::AUDIO_STATUS_E Audio::getStatus()
{
    return sg_audio_status;
}

// Set audio status (private)
void Audio::setStatus(AUDIO_STATUS_E status)
{
    sg_audio_status = status;
}

// Check if idle
bool Audio::isIdle()
{
    return sg_audio_status == AUDIO_STATUS_IDLE;
}

// Check if recording
bool Audio::isRecording()
{
    return sg_audio_status == AUDIO_STATUS_RECORDING;
}

// Check if playing
bool Audio::isPlaying()
{
    return sg_audio_status == AUDIO_STATUS_PLAYING;
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
uint32_t Audio::readRecordedData(uint8_t* buffer, uint32_t len)
{
    if (sg_recorder_pcm_rb == NULL || buffer == NULL) {
        return 0;
    }

    uint32_t available = tuya_ring_buff_used_size_get(sg_recorder_pcm_rb);
    uint32_t to_read = (len < available) ? len : available;
    if (to_read > 0) {
        tuya_ring_buff_read(sg_recorder_pcm_rb, buffer, to_read);
    }

    return to_read;
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

// Static callback wrapper (not used directly, kept for compatibility)
void Audio::audio_frame_callback_wrapper(TDL_AUDIO_FRAME_FORMAT_E type, 
                                        TDL_AUDIO_STATUS_E status,
                                        uint8_t* data, 
                                        uint32_t len)
{
    if (g_audio_instance != NULL && g_audio_instance->frame_callback != NULL) {
        g_audio_instance->frame_callback(data, len);
    }
}
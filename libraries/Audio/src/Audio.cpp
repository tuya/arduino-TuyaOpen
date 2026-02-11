/**
 * @file Audio.cpp
 * @brief Arduino Audio class implementation for Tuya Open platform
 * @version 2.0
 * @copyright Copyright (c) 2021-2026 Tuya Inc. All Rights Reserved.
 */

#include "Audio.h"

#include "tuya_ringbuf.h"
#include "tkl_output.h"

#include "tdl_audio_manage.h"
#include "tdd_audio.h"

// Internal implementation structure (hidden from users)
struct AudioImpl {
    void* audioHandle;              // TDL_AUDIO_HANDLE_T
    bool initialized;
    
    // Audio info
    uint32_t frameSize;
    uint32_t sampleTimeMs;
    
    // Status
    AudioStatus micStatus;
    AudioStatus spkStatus;
    
    // Ring buffer for microphone only
    void* micRingBuffer;            // TUYA_RINGBUFF_T for microphone
    uint32_t micBufferSize;
    
    // Configuration
    uint8_t volume;
    bool enableAEC;
    
    // Callback
    AudioFrameCallback userCallback;
    
    // Playback control
    bool stopPlayFlag;
};

// Global instance pointer for C callback
static Audio* g_audioInstance = nullptr;

// Internal C callback for audio frames
static void audioFrameCallbackInternal(TDL_AUDIO_FRAME_FORMAT_E type,
                                       TDL_AUDIO_STATUS_E status,
                                       uint8_t* data,
                                       uint32_t len)
{
    if (g_audioInstance == nullptr || g_audioInstance->getImpl() == nullptr) {
        return;
    }
    
    AudioImpl* impl = static_cast<AudioImpl*>(g_audioInstance->getImpl());
    
    // Write to microphone ring buffer if recording
    if (impl->micStatus == AudioStatus::RECORDING && impl->micRingBuffer != nullptr) {
        TUYA_RINGBUFF_T micBuf = static_cast<TUYA_RINGBUFF_T>(impl->micRingBuffer);
        tuya_ring_buff_write(micBuf, data, len);
    }
    
    // Call user callback if set
    if (impl->userCallback != nullptr) {
        impl->userCallback(data, len);
    }
}

Audio::Audio()
    : _impl(nullptr)
{
    g_audioInstance = this;
}

Audio::~Audio()
{
    end();
    if (g_audioInstance == this) {
        g_audioInstance = nullptr;
    }
}

OPERATE_RET Audio::begin(const AudioConfig* config)
{
    if (_impl != nullptr && static_cast<AudioImpl*>(_impl)->initialized) {
        PR_WARN("Audio already initialized");
        return OPRT_OK;
    }
    
    // Allocate implementation structure
    _impl = tal_psram_malloc(sizeof(AudioImpl));
    if (_impl == nullptr) {
        PR_ERR("Failed to allocate audio implementation");
        return OPRT_MALLOC_FAILED;
    }
    
    AudioImpl* impl = static_cast<AudioImpl*>(_impl);
    memset(impl, 0, sizeof(AudioImpl));
    
    // Apply configuration
    impl->micBufferSize = (config && config->micBufferSize > 0) ? config->micBufferSize : 16000;
    impl->volume = (config && config->volume <= 100) ? config->volume : 60;
    impl->enableAEC = (config) ? config->enableAEC : false;
    impl->micStatus = AudioStatus::IDLE;
    impl->spkStatus = AudioStatus::IDLE;
    impl->stopPlayFlag = false;
    impl->userCallback = nullptr;
    
#if defined(AUDIO_CODEC_NAME) && defined(PLATFORM_T5) && defined(ARDUINO_AUDIO_CODEC_NAME)
    // Register audio hardware
    TDD_AUDIO_T5AI_T cfg;
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
    cfg.spk_pin_polarity = TUYA_GPIO_LEVEL_LOW;
    
    static char codec_name[] = ARDUINO_AUDIO_CODEC_NAME;
    OPERATE_RET rt = tdd_audio_register(codec_name, cfg);
    if (rt != OPRT_OK) {
        PR_ERR("Failed to register audio codec: %d", rt);
        tal_psram_free(_impl);
        _impl = nullptr;
        return rt;
    }
#endif
    
    // Find and open audio device
    static char audio_name[] = ARDUINO_AUDIO_CODEC_NAME;
    TDL_AUDIO_HANDLE_T audioHdl = nullptr;
    rt = tdl_audio_find(audio_name, &audioHdl);
    if (rt != OPRT_OK) {
        PR_ERR("Failed to find audio device: %d", rt);
        tal_psram_free(_impl);
        _impl = nullptr;
        return rt;
    }
    
    impl->audioHandle = audioHdl;
    
    rt = tdl_audio_open(static_cast<TDL_AUDIO_HANDLE_T>(impl->audioHandle), 
                        audioFrameCallbackInternal);
    if (rt != OPRT_OK) {
        PR_ERR("Failed to open audio device: %d", rt);
        tal_psram_free(_impl);
        _impl = nullptr;
        return rt;
    }
    
    // Get audio info
    TDL_AUDIO_INFO_T audioInfo;
    rt = tdl_audio_get_info(static_cast<TDL_AUDIO_HANDLE_T>(impl->audioHandle), &audioInfo);
    if (rt != OPRT_OK || audioInfo.frame_size == 0 || audioInfo.sample_tm_ms == 0) {
        PR_ERR("Failed to get valid audio info");
        tdl_audio_close(static_cast<TDL_AUDIO_HANDLE_T>(impl->audioHandle));
        tal_psram_free(_impl);
        _impl = nullptr;
        return rt;
    }
    
    impl->frameSize = audioInfo.frame_size;
    impl->sampleTimeMs = audioInfo.sample_tm_ms;
    
    // Create microphone ring buffer
    TUYA_RINGBUFF_T micBuf = nullptr;
    rt = tuya_ring_buff_create(impl->micBufferSize, 
                               (RINGBUFF_TYPE_E)OVERFLOW_PSRAM_STOP_TYPE, 
                               &micBuf);
    if (rt != OPRT_OK) {
        PR_ERR("Failed to create microphone ring buffer: %d", rt);
        tdl_audio_close(static_cast<TDL_AUDIO_HANDLE_T>(impl->audioHandle));
        tal_psram_free(_impl);
        _impl = nullptr;
        return rt;
    }
    impl->micRingBuffer = micBuf;
    
    // Set volume
    setVolume(impl->volume);
    
    impl->initialized = true;
    g_audioInstance = this;
    
    PR_NOTICE("Audio initialized: mic=%dB, vol=%d", 
             impl->micBufferSize, impl->volume);
    
    return OPRT_OK;
}

void Audio::end()
{
    if (_impl == nullptr) {
        return;
    }
    
    AudioImpl* impl = static_cast<AudioImpl*>(_impl);
    
    if (impl->audioHandle != nullptr) {
        tdl_audio_close(static_cast<TDL_AUDIO_HANDLE_T>(impl->audioHandle));
        impl->audioHandle = nullptr;
    }
    
    if (impl->micRingBuffer != nullptr) {
        tuya_ring_buff_free(static_cast<TUYA_RINGBUFF_T>(impl->micRingBuffer));
        impl->micRingBuffer = nullptr;
    }
    
    tal_psram_free(_impl);
    _impl = nullptr;
    
    PR_NOTICE("Audio closed");
}

OPERATE_RET Audio::startRecord()
{
    if (_impl == nullptr) {
        PR_ERR("Audio not initialized");
        return OPRT_COM_ERROR;
    }
    
    AudioImpl* impl = static_cast<AudioImpl*>(_impl);
    
    if (impl->micRingBuffer == nullptr) {
        PR_ERR("Microphone buffer not available");
        return OPRT_COM_ERROR;
    }
    
    // Reset microphone buffer
    tuya_ring_buff_reset(static_cast<TUYA_RINGBUFF_T>(impl->micRingBuffer));
    
    // Update status
    impl->micStatus = AudioStatus::RECORDING;
    
    PR_NOTICE("Recording started");
    return OPRT_OK;
}

void Audio::stopRecord()
{
    if (_impl == nullptr) {
        return;
    }
    
    AudioImpl* impl = static_cast<AudioImpl*>(_impl);
    
    if (impl->micStatus == AudioStatus::RECORDING) {
        impl->micStatus = AudioStatus::IDLE;
        PR_NOTICE("Recording stopped");
    }
}

OPERATE_RET Audio::play(const uint8_t* data, uint32_t len)
{
    if (_impl == nullptr || data == nullptr || len == 0) {
        PR_ERR("Invalid parameters for play");
        return OPRT_INVALID_PARM;
    }
    
    AudioImpl* impl = static_cast<AudioImpl*>(_impl);
    
    if (impl->spkStatus != AudioStatus::PLAYING) {
        impl->spkStatus = AudioStatus::PLAYING;
    }
    
    OPERATE_RET rt = tdl_audio_play(static_cast<TDL_AUDIO_HANDLE_T>(impl->audioHandle), 
                                    const_cast<uint8_t*>(data), len);
    return rt;
}

OPERATE_RET Audio::playRecordedData()
{
    if (_impl == nullptr) {
        PR_ERR("Audio not initialized");
        return OPRT_COM_ERROR;
    }
    
    AudioImpl* impl = static_cast<AudioImpl*>(_impl);
    
    if (impl->micRingBuffer == nullptr || impl->frameSize == 0) {
        PR_ERR("Microphone buffer not properly initialized");
        return OPRT_COM_ERROR;
    }
    
    TUYA_RINGBUFF_T micBuf = static_cast<TUYA_RINGBUFF_T>(impl->micRingBuffer);
    
    // Get data length from mic buffer
    uint32_t micDataLen = tuya_ring_buff_used_size_get(micBuf);
    if (micDataLen == 0) {
        PR_NOTICE("No recorded data to play");
        return OPRT_OK;
    }
    
    // Allocate frame buffer
    PR_DEBUG("Allocating frame buffer: %d bytes", impl->frameSize);
    uint8_t* frameBuf = (uint8_t*)tal_psram_malloc(impl->frameSize);
    if (frameBuf == nullptr) {
        PR_ERR("Failed to allocate frame buffer");
        return OPRT_MALLOC_FAILED;
    }
    
    // Start playback
    impl->spkStatus = AudioStatus::PLAYING;
    impl->stopPlayFlag = false;
    
    PR_NOTICE("Playback started: %d bytes", micDataLen);
    
    // Playback loop: read frame from mic buffer and play directly
    while (!impl->stopPlayFlag) {
        uint32_t available = tuya_ring_buff_used_size_get(micBuf);
        if (available == 0) {
            break;
        }
        
        memset(frameBuf, 0, impl->frameSize);
        
        uint32_t toRead = (available > impl->frameSize) ? 
                         impl->frameSize : available;
        
        tuya_ring_buff_read(micBuf, frameBuf, toRead);
        tdl_audio_play(static_cast<TDL_AUDIO_HANDLE_T>(impl->audioHandle), 
                      frameBuf, toRead);
    }
    
    tal_psram_free(frameBuf);
    
    impl->spkStatus = AudioStatus::IDLE;
    
    PR_NOTICE("Playback completed");
    return OPRT_OK;
}

void Audio::stopPlay()
{
    if (_impl == nullptr) {
        return;
    }
    
    AudioImpl* impl = static_cast<AudioImpl*>(_impl);
    
    impl->stopPlayFlag = true;
    impl->spkStatus = AudioStatus::IDLE;
    
    PR_NOTICE("Playback stopped");
}

OPERATE_RET Audio::setVolume(uint8_t volume)
{
    if (_impl == nullptr) {
        return OPRT_COM_ERROR;
    }
    
    AudioImpl* impl = static_cast<AudioImpl*>(_impl);
    
    if (volume > 100) {
        volume = 100;
    }
    
    impl->volume = volume;
    
    if (impl->audioHandle != nullptr) {
        OPERATE_RET rt = tdl_audio_volume_set(
            static_cast<TDL_AUDIO_HANDLE_T>(impl->audioHandle), volume);
        if (rt == OPRT_OK) {
            PR_NOTICE("Volume set to %d", volume);
        }
        return rt;
    }
    
    return OPRT_OK;
}

uint8_t Audio::getVolume() const
{
    if (_impl == nullptr) {
        return 0;
    }
    
    return static_cast<AudioImpl*>(_impl)->volume;
}

AudioStatus Audio::getMicStatus() const
{
    if (_impl == nullptr) {
        return AudioStatus::IDLE;
    }
    
    return static_cast<AudioImpl*>(_impl)->micStatus;
}

AudioStatus Audio::getSpkStatus() const
{
    if (_impl == nullptr) {
        return AudioStatus::IDLE;
    }
    
    return static_cast<AudioImpl*>(_impl)->spkStatus;
}

bool Audio::isIdle() const
{
    if (_impl == nullptr) {
        return true;
    }
    
    AudioImpl* impl = static_cast<AudioImpl*>(_impl);
    return (impl->micStatus == AudioStatus::IDLE && 
            impl->spkStatus == AudioStatus::IDLE);
}

bool Audio::isRecording() const
{
    if (_impl == nullptr) {
        return false;
    }
    
    return static_cast<AudioImpl*>(_impl)->micStatus == AudioStatus::RECORDING;
}

bool Audio::isPlaying() const
{
    if (_impl == nullptr) {
        return false;
    }
    
    return static_cast<AudioImpl*>(_impl)->spkStatus == AudioStatus::PLAYING;
}

uint32_t Audio::getRecordedDataLen() const
{
    if (_impl == nullptr) {
        return 0;
    }
    
    AudioImpl* impl = static_cast<AudioImpl*>(_impl);
    
    if (impl->micRingBuffer == nullptr) {
        return 0;
    }
    
    return tuya_ring_buff_used_size_get(
        static_cast<TUYA_RINGBUFF_T>(impl->micRingBuffer));
}

uint32_t Audio::readRecordedData(uint8_t* buffer, uint32_t len)
{
    if (_impl == nullptr || buffer == nullptr) {
        return 0;
    }
    
    AudioImpl* impl = static_cast<AudioImpl*>(_impl);
    
    if (impl->micRingBuffer == nullptr) {
        return 0;
    }
    
    TUYA_RINGBUFF_T micBuf = static_cast<TUYA_RINGBUFF_T>(impl->micRingBuffer);
    
    uint32_t available = tuya_ring_buff_used_size_get(micBuf);
    uint32_t toRead = (len < available) ? len : available;
    
    if (toRead > 0) {
        tuya_ring_buff_read(micBuf, buffer, toRead);
    }
    
    return toRead;
}

void Audio::clearRecordedData()
{
    if (_impl == nullptr) {
        return;
    }
    
    AudioImpl* impl = static_cast<AudioImpl*>(_impl);
    
    if (impl->micRingBuffer != nullptr) {
        tuya_ring_buff_reset(static_cast<TUYA_RINGBUFF_T>(impl->micRingBuffer));
        PR_NOTICE("Recorded data cleared");
    }
}

void Audio::setAudioFrameCallback(AudioFrameCallback callback)
{
    if (_impl == nullptr) {
        return;
    }
    
    AudioImpl* impl = static_cast<AudioImpl*>(_impl);
    impl->userCallback = callback;
}

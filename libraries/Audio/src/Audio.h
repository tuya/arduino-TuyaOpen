/**
 * @file Audio.h
 * @brief Arduino Audio class for Tuya Open platform
 * @version 2.0
 * @copyright Copyright (c) 2021-2026 Tuya Inc. All Rights Reserved.
 */

#ifndef __AUDIO_H__
#define __AUDIO_H__

#include "tuya_cloud_types.h"
#include "tal_api.h"

#include "tkl_memory.h"
#include "board_com_api.h"
/***********************************************************
************************macro define************************
***********************************************************/
#define ARDUINO_AUDIO_CODEC_NAME    "InoAudioCodec"

/**
 * @brief Audio status enumeration
 */
enum class AudioStatus {
    IDLE = 0,           /**< Idle, ready for operation */
    RECORD_START,       /**< Recording started */
    RECORDING,          /**< Recording in progress */
    RECORD_END,         /**< Recording ended */
    PLAY_START,         /**< Playback started */
    PLAYING,            /**< Playback in progress */
    PLAY_END            /**< Playback ended */
};

/**
 * @brief Audio configuration structure
 */
struct AudioConfig {
    uint32_t micBufferSize;     /**< Microphone buffer size in bytes (size = xxx ms / 10 * 640) */
    uint8_t volume;             /**< Initial volume (0-100, default 60) */
    uint8_t spkPin;             /**< Speaker control pin */
    bool enableAEC;             /**< Enable acoustic echo cancellation */
    
    AudioConfig() : micBufferSize(0), volume(60), 
                    spkPin(28), enableAEC(true) {}
};

/**
 * @brief Audio frame callback function type
 * @param data Pointer to audio frame data
 * @param len Length of audio frame data in bytes
 */
typedef void (*AudioFrameCallback)(uint8_t* data, uint32_t len);

/**
 * @brief Audio class for Arduino Tuya Open platform
 */
class Audio {
public:
    /**
     * @brief Constructor
     */
    Audio();

    /**
     * @brief Destructor
     */
    ~Audio();

    /**
     * @brief Initialize audio with default or custom configuration
     * @param config Audio configuration (NULL for defaults)
     * @return OPRT_OK on success, error code otherwise
     */
    OPERATE_RET begin(const AudioConfig* config = nullptr);

    /**
     * @brief Stop audio and release resources
     */
    void end();

    /**
     * @brief Start recording audio from microphone
     * @return OPRT_OK on success, error code otherwise
     */
    OPERATE_RET startRecord();

    /**
     * @brief Stop recording
     */
    void stopRecord();

    /**
     * @brief Play audio data directly to speaker
     * @param data Pointer to audio data
     * @param len Length of audio data in bytes
     * @return OPRT_OK on success, error code otherwise
     */
    OPERATE_RET play(const uint8_t* data, uint32_t len);

    /**
     * @brief Play recorded data from microphone buffer
     * @return OPRT_OK on success, error code otherwise
     * @note This copies data from mic buffer to speaker buffer and plays it
     */
    OPERATE_RET playRecordedData();

    /**
     * @brief Stop playback
     */
    void stopPlay();

    /**
     * @brief Set speaker volume
     * @param volume Volume level (0-100)
     * @return OPRT_OK on success, error code otherwise
     */
    OPERATE_RET setVolume(uint8_t volume);

    /**
     * @brief Get current volume
     * @return Current volume (0-100)
     */
    uint8_t getVolume() const;

    /**
     * @brief Get microphone status
     * @return Current microphone status
     */
    AudioStatus getMicStatus() const;

    /**
     * @brief Get speaker status
     * @return Current speaker status
     */
    AudioStatus getSpkStatus() const;

    /**
     * @brief Check if audio is idle
     * @return true if both mic and speaker are idle
     */
    bool isIdle() const;

    /**
     * @brief Check if recording
     * @return true if microphone is recording
     */
    bool isRecording() const;

    /**
     * @brief Check if playing
     * @return true if speaker is playing
     */
    bool isPlaying() const;

    /**
     * @brief Get length of recorded data in microphone buffer
     * @return Length in bytes
     */
    uint32_t getRecordedDataLen() const;

    /**
     * @brief Read recorded data from microphone buffer
     * @param buffer Buffer to store data
     * @param len Maximum length to read
     * @return Actual bytes read
     */
    uint32_t readRecordedData(uint8_t* buffer, uint32_t len);

    /**
     * @brief Clear all recorded data from microphone buffer
     */
    void clearRecordedData();

    /**
     * @brief Set callback for real-time audio frame processing
     * @param callback Callback function (nullptr to disable)
     */
    void setAudioFrameCallback(AudioFrameCallback callback);

    // Internal helper for callbacks (public for C callback access)
    void* getImpl() { return _impl; }

private:
    // Internal implementation (opaque pointer)
    void *_impl;
};

#endif // __AUDIO_H__
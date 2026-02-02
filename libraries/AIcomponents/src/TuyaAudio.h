/**
 * @file TuyaAudio.h
 * @author Tuya Inc.
 * @brief TuyaAudioClass declaration and Audio-related types
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 */
#ifndef __TUYA_AUDIO_H_
#define __TUYA_AUDIO_H_
#include "tuya_cloud_types.h"

#include "svc_ai_player.h"
#include "ai_audio_player.h"

/***********************************************************
***********************class definition*********************
***********************************************************/

/**
 * @class TuyaAudioClass
 * @brief Nested class for audio management
 * 
 * Provides interface for audio input/output control including
 * volume, playback, and recording.
 */
class TuyaAudioClass {
public:
    TuyaAudioClass();
    ~TuyaAudioClass();
    
    /**
     * @brief Initialize audio system
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET begin(uint8_t spkPin = 28);
    
    /**
     * @brief Deinitialize audio system
     */
    void end();
    
    /**
     * @brief Check if audio is initialized
     * @return true if initialized
     */
    bool isInitialized();
    
    //==========================================================================
    // Volume Control
    //==========================================================================
    
    /**
     * @brief Set audio volume
     * @param volume Volume value (0-100)
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET setVolume(int volume);
    
    /**
     * @brief Get current audio volume
     * @return Current volume (0-100)
     */
    int getVolume();
    
    /**
     * @brief Increase volume by step
     * @param step Volume step (default 10)
     * @return New volume value
     */
    int volumeUp(int step = 10);
    
    /**
     * @brief Decrease volume by step
     * @param step Volume step (default 10)
     * @return New volume value
     */
    int volumeDown(int step = 10);
    
    /**
     * @brief Mute audio
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET mute();
    
    /**
     * @brief Unmute audio
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET unmute();
    
    /**
     * @brief Check if audio is muted
     * @return true if muted
     */
    bool isMuted();
    
    //==========================================================================
    // Playback Control
    //==========================================================================
    
    /**
     * @brief Stop current audio playback
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET stop();
    
    /**
     * @brief Pause current audio playback
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET pause();
    
    /**
     * @brief Resume paused audio playback
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET resume();
    
    /**
     * @brief Check if audio is currently playing
     * @return true if playing
     */
    bool isPlaying();
    
    /**
     * @brief Play alert sound
     * @param type Alert type to play (AI_AUDIO_ALERT_TYPE_E from ai_audio_player.h)
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET playAlert(AI_AUDIO_ALERT_TYPE_E type);
    
    /**
     * @brief Play audio from URL
     * @param url Audio URL
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET playUrl(const char *url);
    
    /**
     * @brief Play audio from memory buffer
     * @param data Audio data buffer
     * @param len  Data length
     * @param format Audio format (AI_AUDIO_CODEC_E from svc_ai_player.h)
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET playData(uint8_t *data, uint32_t len, AI_AUDIO_CODEC_E format = AI_AUDIO_CODEC_MP3);
    
    //==========================================================================
    // Recording Control
    //==========================================================================
    
    /**
     * @brief Start voice input recording
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET startRecording();
    
    /**
     * @brief Stop voice input recording
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET stopRecording();
    
    /**
     * @brief Check if currently recording
     * @return true if recording
     */
    bool isRecording();

private:
    bool _initialized;
    int  _volume;
    int  _prevVolume;
    bool _muted;
    bool _recording;
    
    friend class TuyaAIClass;  // Allow TuyaAIClass to access _initialized
};

#endif /* __TUYA_AUDIO_H_ */

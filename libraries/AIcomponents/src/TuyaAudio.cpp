/**
 * @file TuyaAudio.cpp
 * @author Tuya Inc.
 * @brief TuyaAudioClass implementation - Audio input/output management
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 */
#include "TuyaAI.h"
#include <string.h>

#include "tkl_kws.h"
#include "tdd_audio.h"
#include "tdl_audio_manage.h"

#include "ai_chat_main.h"
#if defined(ENABLE_COMP_AI_AUDIO) && (ENABLE_COMP_AI_AUDIO == 1)
#include "ai_audio_player.h"
#include "ai_audio_input.h"
#endif

extern "C" { 
#include "tuya_ai_agent.h"
}
/***********************************************************
************************TuyaAudioClass Definition***********
***********************************************************/
#define AI_AUDIO_SLICE_TIME         80     
#define AI_AUDIO_VAD_ACTIVE_TIME    200 
#define AI_AUDIO_VAD_OFF_TIME       1000
#define _SPEAKER_EN_PIN             TUYA_GPIO_NUM_28

#if defined(ENABLE_COMP_AI_AUDIO) && (ENABLE_COMP_AI_AUDIO == 1)
/**
@brief Audio output callback function
@param data Pointer to audio data
@param datalen Audio data length
@return int Operation result
*/
static int __ai_audio_output(uint8_t *data, uint16_t datalen)
{
    OPERATE_RET rt = OPRT_OK;
    uint64_t   pts = 0;
    uint64_t   timestamp = 0;

    // if(false == sg_ai_agent_inited) {
    //     return OPRT_OK;
    // }

#if defined(ENABLE_AUDIO_AEC) && (ENABLE_AUDIO_AEC == 1)
    timestamp = pts = tal_system_get_millisecond();
    TUYA_CALL_ERR_LOG(tuya_ai_audio_input(timestamp, pts, data, datalen, datalen));
#else 
    if(false == ai_audio_player_is_playing()) {
        timestamp = pts = tal_system_get_millisecond();
        TUYA_CALL_ERR_LOG(tuya_ai_audio_input(timestamp, pts, data, datalen, datalen));
    }
#endif

    return rt;
}

/**
@brief Handle VAD (Voice Activity Detection) state change event callback
@param data Pointer to VAD state value (AI_AUDIO_VAD_STATE_E)
@return OPERATE_RET Operation result
*/
int __ai_vad_change_evt(void *data)
{
    OPERATE_RET rt = OPRT_OK;

    TUYA_CHECK_NULL_RETURN(data, OPRT_INVALID_PARM);

    AI_AUDIO_VAD_STATE_E vad_flag = (AI_AUDIO_VAD_STATE_E)(uintptr_t)data;
    PR_DEBUG("VAD: %s, vad_flag: %d", vad_flag == AI_AUDIO_VAD_START ? "START" : "STOP", vad_flag);

    TUYA_CALL_ERR_RETURN(ai_mode_vad_change(vad_flag));

    return rt;
}

#endif

/***********************************************************
***********************TuyaAudioClass Implementation********
***********************************************************/

TuyaAudioClass::TuyaAudioClass() {
    _initialized = false;
    _volume = TUYA_AI_DEFAULT_VOLUME;
    _prevVolume = TUYA_AI_DEFAULT_VOLUME;
    _muted = false;
    _recording = false;
}

TuyaAudioClass::~TuyaAudioClass() {
    if (_initialized) {
        end();
    }
}

OPERATE_RET TuyaAudioClass::begin() {
    if (_initialized) {
        return OPRT_OK;
    }
    OPERATE_RET rt = OPRT_OK;
    
    // Audio is typically initialized as part of ai_chat_init
    AI_AUDIO_INPUT_CFG_T input_cfg= {
        .vad_mode      = AI_AUDIO_VAD_MANUAL,
        .vad_off_ms    = AI_AUDIO_VAD_OFF_TIME,
        .vad_active_ms = AI_AUDIO_VAD_ACTIVE_TIME,
        .slice_ms      = AI_AUDIO_SLICE_TIME,
        .output_cb     = __ai_audio_output,
    };
    TUYA_CALL_ERR_RETURN(ai_audio_input_init(&input_cfg));

    TUYA_CALL_ERR_RETURN(ai_audio_player_init());

    TUYA_CALL_ERR_RETURN(tkl_kws_init());

    int vol = ai_chat_get_volume();
    TUYA_CALL_ERR_LOG(ai_audio_player_set_vol(vol));

    TUYA_CALL_ERR_RETURN(tal_event_subscribe(EVENT_AUDIO_VAD, "vad_change", __ai_vad_change_evt, SUBSCRIBE_TYPE_NORMAL));
    PR_NOTICE("TuyaAI Audio initialized");
    // This method is for standalone audio initialization if needed
    _initialized = true;
    return OPRT_OK;
}

void TuyaAudioClass::end() {
    if (!_initialized) {
        return;
    }
    
#if defined(ENABLE_COMP_AI_AUDIO) && (ENABLE_COMP_AI_AUDIO == 1)
    ai_audio_player_deinit();
    ai_audio_input_deinit();
#endif
    
    _initialized = false;
}

bool TuyaAudioClass::isInitialized() {
    return _initialized;
}

OPERATE_RET TuyaAudioClass::setVolume(int volume) {
    if (volume < TUYA_AI_MIN_VOLUME) volume = TUYA_AI_MIN_VOLUME;
    if (volume > TUYA_AI_MAX_VOLUME) volume = TUYA_AI_MAX_VOLUME;
    
    OPERATE_RET rt = ai_chat_set_volume(volume);
    if (rt == OPRT_OK) {
        _volume = volume;
        if (!_muted) {
            _prevVolume = volume;
        }
    }
    return rt;
}

int TuyaAudioClass::getVolume() {
    _volume = ai_chat_get_volume();
    return _volume;
}

int TuyaAudioClass::volumeUp(int step) {
    int newVol = getVolume() + step;
    setVolume(newVol);
    _muted = false;
    return getVolume();
}

int TuyaAudioClass::volumeDown(int step) {
    int newVol = getVolume() - step;
    setVolume(newVol);
    return getVolume();
}

OPERATE_RET TuyaAudioClass::mute() {
    if (!_muted) {
        _prevVolume = getVolume();
        _muted = true;
        return setVolume(0);
    }
    return OPRT_OK;
}

OPERATE_RET TuyaAudioClass::unmute() {
    if (_muted) {
        _muted = false;
        return setVolume(_prevVolume);
    }
    return OPRT_OK;
}

bool TuyaAudioClass::isMuted() {
    return _muted;
}

OPERATE_RET TuyaAudioClass::stop() {
#if defined(ENABLE_COMP_AI_AUDIO) && (ENABLE_COMP_AI_AUDIO == 1)
    return ai_audio_player_stop(AI_AUDIO_PLAYER_ALL);
#else
    return OPRT_NOT_SUPPORTED;
#endif
}

OPERATE_RET TuyaAudioClass::pause() {
#if defined(ENABLE_COMP_AI_AUDIO) && (ENABLE_COMP_AI_AUDIO == 1)
    return ai_audio_player_set_resume(false);
#else
    return OPRT_NOT_SUPPORTED;
#endif
}

OPERATE_RET TuyaAudioClass::resume() {
#if defined(ENABLE_COMP_AI_AUDIO) && (ENABLE_COMP_AI_AUDIO == 1)
    return ai_audio_player_set_resume(true);
#else
    return OPRT_NOT_SUPPORTED;
#endif
}

bool TuyaAudioClass::isPlaying() {
#if defined(ENABLE_COMP_AI_AUDIO) && (ENABLE_COMP_AI_AUDIO == 1)
    return ai_audio_player_is_playing() != 0;
#else
    return false;
#endif
}

OPERATE_RET TuyaAudioClass::playAlert(AI_AUDIO_ALERT_TYPE_E type) {
#if defined(ENABLE_COMP_AI_AUDIO) && (ENABLE_COMP_AI_AUDIO == 1)
    return ai_audio_player_alert(type);
#else
    return OPRT_NOT_SUPPORTED;
#endif
}

OPERATE_RET TuyaAudioClass::playUrl(const char *url) {
#if defined(ENABLE_COMP_AI_AUDIO) && (ENABLE_COMP_AI_AUDIO == 1)
    if (url == nullptr) return OPRT_INVALID_PARM;
    
    AI_AUDIO_MUSIC_T music = {0};
    AI_MUSIC_SRC_T src = {0};
    
    src.url = (char *)url;
    strncpy(music.action, "play", sizeof(music.action) - 1);
    music.src_cnt = 1;
    music.src_array = &src;
    
    return ai_audio_play_music(&music);
#else
    return OPRT_NOT_SUPPORTED;
#endif
}

OPERATE_RET TuyaAudioClass::playData(uint8_t *data, uint32_t len, AI_AUDIO_CODEC_E format) {
#if defined(ENABLE_COMP_AI_AUDIO) && (ENABLE_COMP_AI_AUDIO == 1)
    if (data == nullptr || len == 0) return OPRT_INVALID_PARM;
    return ai_audio_play_data(format, data, len);
#else
    return OPRT_NOT_SUPPORTED;
#endif
}

OPERATE_RET TuyaAudioClass::startRecording() {
#if defined(ENABLE_COMP_AI_AUDIO) && (ENABLE_COMP_AI_AUDIO == 1)
    OPERATE_RET rt = ai_audio_input_start();
    if (rt == OPRT_OK) {
        _recording = true;
    }
    return rt;
#else
    return OPRT_NOT_SUPPORTED;
#endif
}

OPERATE_RET TuyaAudioClass::stopRecording() {
#if defined(ENABLE_COMP_AI_AUDIO) && (ENABLE_COMP_AI_AUDIO == 1)
    _recording = false;
    return ai_audio_input_stop();
#else
    return OPRT_NOT_SUPPORTED;
#endif
}

bool TuyaAudioClass::isRecording() {
    return _recording;
}

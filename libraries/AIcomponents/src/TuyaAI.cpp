/**
 * @file TuyaAI.cpp
 * @author Tuya Inc.
 * @brief TuyaAI Arduino C++ wrapper main implementation
 *
 * This source file provides the implementation of the TuyaAIClass
 * and shared utility functions. Nested class implementations are
 * in separate files: TuyaUI.cpp, TuyaAudio.cpp, TuyaMCP.cpp
 *
 * @note Button handling is NOT included - use Button class separately
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 *
 */
#include "TuyaAI.h"
#include <string.h>

extern "C" {
#include "tuya_ai_agent.h"

extern OPERATE_RET __ai_chat_save_config(uint32_t mode, int volume);
extern OPERATE_RET __ai_chat_load_config(uint32_t *mode, int *volume);
}
#include "ai_main/include/ai_chat_main.h"
#include "ai_agent/include/ai_agent.h"
#include "ai_mode/include/ai_manage_mode.h"
#include "utility/include/ai_user_event.h"
#include "lang_config.h"

#if defined(ENABLE_COMP_AI_AUDIO) && (ENABLE_COMP_AI_AUDIO == 1)
#include "ai_audio/include/ai_audio_player.h"
#include "ai_audio/include/ai_audio_input.h"
#endif

#include "tdl_button_manage.h"
/***********************************************************
************************static variables********************
***********************************************************/
// Singleton instance
TuyaAIClass TuyaAI;

// Static callback storage for internal event routing
static TuyaAIClass *_aiInstance = nullptr;

/***********************************************************
***********************static function declarations*********
***********************************************************/

// Forward declarations for static functions
static void _internalEventHandler(AI_NOTIFY_EVENT_T *event);
static AI_CHAT_MODE_E _convertToInternalMode(AIChatMode_t mode);
static AIChatMode_t _convertFromInternalMode(AI_CHAT_MODE_E mode);
// Note: _convertToInternalAlert moved to TuyaAudio.cpp

/***********************************************************
***********************TuyaAIClass Implementation***********
***********************************************************/

TuyaAIClass::TuyaAIClass() {
    _initialized = false;
    _chatMode = AI_MODE_WAKEUP;
    _eventCallback = nullptr;
    _stateCallback = nullptr;
    _alertCallback = nullptr;
    _userArg = nullptr;
    _aiInstance = this;
}

TuyaAIClass::~TuyaAIClass() {
    if (_initialized) {
        end();
    }
    _aiInstance = nullptr;
}

OPERATE_RET TuyaAIClass::begin(AIConfig_t &config) {
    if (_initialized) {
        return OPRT_OK;
    }
    
    OPERATE_RET rt = OPRT_OK;
    
    // Store configuration
    _chatMode = config.chatMode;
    _eventCallback = config.eventCb;
    _stateCallback = config.stateCb;
    _userArg = config.userArg;
    
    // Convert to internal chat mode
    AI_CHAT_MODE_E internalMode = _convertToInternalMode(_chatMode);
    
    // Setup chat mode configuration
    AI_CHAT_MODE_CFG_T aiChatCfg = {
        .default_mode = internalMode,
        .default_vol  = config.volume,
        .evt_cb       = _internalEventHandler,
    };
    
    // Initialize AI chat module
    rt = ai_chat_init(&aiChatCfg);
    if (rt != OPRT_OK) {
        return rt;
    }
    
    _initialized = true;
    return OPRT_OK;
}

OPERATE_RET TuyaAIClass::begin() {
    AIConfig_t config = {
        .chatMode = AI_MODE_WAKEUP,
        .volume = TUYA_AI_DEFAULT_VOLUME,
        .eventCb = nullptr,
        .stateCb = nullptr,
        .userArg = nullptr
    };
    return begin(config);
}

void TuyaAIClass::end() {
    if (!_initialized) {
        return;
    }
    
    // Deinitialize MCP
    MCP.end();
    
    // Deinitialize UI
    UI.end();
    
    // Deinitialize Audio
    Audio.end();
    
    // Deinitialize AI components
    ai_agent_deinit();
    ai_mode_deinit();
    
    _initialized = false;
}

bool TuyaAIClass::isInitialized() {
    return _initialized;
}

OPERATE_RET TuyaAIClass::snedText(const char *text) {
    if (text == nullptr) return OPRT_INVALID_PARM;
    return ai_agent_send_text((char *)text);
}

OPERATE_RET TuyaAIClass::snedText(uint8_t *buffer, int len) {
    if (buffer == nullptr || len <= 0) return OPRT_INVALID_PARM;
    
    char *text = (char *)Malloc(len + 1);
    if (text == nullptr) return OPRT_MALLOC_FAILED;
    
    memcpy(text, buffer, len);
    text[len] = '\0';
    
    OPERATE_RET rt = ai_agent_send_text(text);
    Free(text);
    
    return rt;
}

OPERATE_RET TuyaAIClass::startVoiceInput() {
    return Audio.startRecording();
}

OPERATE_RET TuyaAIClass::stopVoiceInput() {
    return Audio.stopRecording();
}

OPERATE_RET TuyaAIClass::sendImage(uint8_t *data, uint32_t len) {
    if (data == nullptr || len == 0) return OPRT_INVALID_PARM;
    return ai_agent_send_image(data, len);
}

OPERATE_RET TuyaAIClass::sendFile(uint8_t *data, uint32_t len) {
    if (data == nullptr || len == 0) return OPRT_INVALID_PARM;
    return ai_agent_send_file(data, len);
}

OPERATE_RET TuyaAIClass::setChatMode(AIChatMode_t mode) {
    AI_CHAT_MODE_E internalMode = _convertToInternalMode(mode);
    OPERATE_RET rt = ai_mode_switch(internalMode);
    if (rt == OPRT_OK) {
        _chatMode = mode;
    }
    return rt;
}

AIChatMode_t TuyaAIClass::getChatMode() {
    AI_CHAT_MODE_E internalMode;
    ai_mode_get_curr_mode(&internalMode);
    return _convertFromInternalMode(internalMode);
}

AIChatMode_t TuyaAIClass::nextChatMode() {
    AI_CHAT_MODE_E internalMode = ai_mode_switch_next();
    _chatMode = _convertFromInternalMode(internalMode);
    return _chatMode;
}

AIState_t TuyaAIClass::getState() {
    AI_MODE_STATE_E internalState = ai_mode_get_state();
    
    switch (internalState) {
        case AI_MODE_STATE_IDLE:   return AI_STATE_STANDBY;
        case AI_MODE_STATE_LISTEN: return AI_STATE_LISTENING;
        case AI_MODE_STATE_UPLOAD: return AI_STATE_UPLOADING;
        case AI_MODE_STATE_THINK:  return AI_STATE_THINKING;
        case AI_MODE_STATE_SPEAK:  return AI_STATE_SPEAKING;
        default:                   return AI_STATE_IDLE;
    }
}

const char* TuyaAIClass::getStateString() {
    return ai_get_mode_state_str(ai_mode_get_state());
}

const char* TuyaAIClass::getModeString() {
    AI_CHAT_MODE_E internalMode;
    ai_mode_get_curr_mode(&internalMode);
    return ai_get_mode_name_str(internalMode);
}

OPERATE_RET TuyaAIClass::saveModeConfig(AIChatMode_t mode, int volume) {
    return __ai_chat_save_config((uint32_t)mode, volume);
}

OPERATE_RET TuyaAIClass::loadModeConfig(AIChatMode_t *mode, int *volume) {
    if (mode == nullptr || volume == nullptr) {
        return OPRT_INVALID_PARM;
    }
    uint32_t temp = 0;
    OPERATE_RET rt = __ai_chat_load_config(&temp, volume);
    *mode = (AIChatMode_t)temp;
    return rt;
}

OPERATE_RET TuyaAIClass::modeKeyHandle(uint8_t event, void *arg) {
    return ai_mode_handle_key((TDL_BUTTON_TOUCH_EVENT_E)event, arg);
}

OPERATE_RET TuyaAIClass::interruptChat() {
    Audio.stop();
    tuya_ai_agent_event(AI_EVENT_CHAT_BREAK, 0);
    return OPRT_OK;
}

OPERATE_RET TuyaAIClass::switchRole(const char *roleName) {
    if (roleName == nullptr) return OPRT_INVALID_PARM;
    return ai_agent_role_switch((char *)roleName);
}

OPERATE_RET TuyaAIClass::requestCloudAlert(AIAlertType_t type) {
    AI_ALERT_TYPE_E internalType = (AI_ALERT_TYPE_E)type;
    return ai_agent_cloud_alert(internalType);
}

void TuyaAIClass::setEventCallback(AIEventCallback_t callback, void *arg) {
    _eventCallback = callback;
    _userArg = arg;
}

void TuyaAIClass::setStateCallback(AIStateCallback_t callback) {
    _stateCallback = callback;
}

void TuyaAIClass::setAlertCallback(AIAlertCallback_t callback) {
    _alertCallback = callback;
}

const char* TuyaAIClass::getLanguageCode() {
    return LANG_CODE;
}

/***********************************************************
***********************Static Functions*********************
***********************************************************/

static void _internalEventHandler(AI_NOTIFY_EVENT_T *event) {
    if (_aiInstance == nullptr || event == nullptr) {
        return;
    }
    
    // Convert internal event to external event type
    AIEvent_t extEvent = (event->type);
    
    // Handle state changes
    if (event->type == AI_USER_EVT_MODE_STATE_UPDATE) {
        AIStateCallback_t stateCb = _aiInstance->getStateCallback();
        if (stateCb != nullptr) {
            AIState_t state = _aiInstance->getState();
            stateCb(state);
        }
    }
    
    // Handle alert events
    if (event->type == AI_USER_EVT_PLAY_ALERT) {
        AIAlertCallback_t alertCb = _aiInstance->getAlertCallback();
        if (alertCb != nullptr) {
            // Let user handle alert first
            if (event->data) {
                AIAlertType_t alertType = (AIAlertType_t)(*(int *)event->data);
                if (alertCb(alertType) == 0) {
                    return;  // User handled the alert
                }
            }
        }
    }
    
    // Forward event to user callback
    AIEventCallback_t eventCb = _aiInstance->getEventCallback();
    if (eventCb != nullptr) {
        uint8_t *data = nullptr;
        uint32_t len = 0;
        
        switch (event->type) {
            // ASR events - pass recognized text data
            case AI_USER_EVT_ASR_OK:
            case AI_USER_EVT_ASR_EMPTY:
                if (event->data) {
                    AI_NOTIFY_TEXT_T *textData = (AI_NOTIFY_TEXT_T *)event->data;
                    data = (uint8_t *)textData->data;
                    len = textData->datalen;
                }
                break;
            
            // Text stream events - pass AI response text data
            case AI_USER_EVT_TEXT_STREAM_START:
            case AI_USER_EVT_TEXT_STREAM_DATA:
            case AI_USER_EVT_TEXT_STREAM_STOP:
                if (event->data) {
                    AI_NOTIFY_TEXT_T *textData = (AI_NOTIFY_TEXT_T *)event->data;
                    data = (uint8_t *)textData->data;
                    len = textData->datalen;
                }
                break;
            
            // TTS audio data event - pass raw audio data (MP3/OPUS)
            case AI_USER_EVT_TTS_DATA:
                if (event->data) {
                    AI_NOTIFY_TEXT_T *textData = (AI_NOTIFY_TEXT_T *)event->data;
                    data = (uint8_t *)textData->data;
                    len = textData->datalen;
                }
                break;
                
            // Emotion events - pass emoji/name structure
            case AI_USER_EVT_EMOTION:
            case AI_USER_EVT_LLM_EMOTION:
                if (event->data) {
                    AI_NOTIFY_EMO_T *emoData = (AI_NOTIFY_EMO_T *)event->data;
                    data = (uint8_t *)emoData;
                    len = sizeof(AI_NOTIFY_EMO_T);
                }
                break;
            
            // MIC data event - pass raw PCM audio data
            case AI_USER_EVT_MIC_DATA:
                if (event->data) {
                    AI_NOTIFY_MIC_DATA_T *micData = (AI_NOTIFY_MIC_DATA_T *)event->data;
                    data = micData->data;
                    len = micData->data_len;
                }
                break;
            
            // Skill event - pass cJSON pointer as data
            case AI_USER_EVT_SKILL:
                // data is cJSON*, pass raw pointer
                data = (uint8_t *)event->data;
                len = event->data ? 1 : 0;  // non-zero len indicates valid data
                break;
                
            default:
                data = (uint8_t *)event->data;
                len = 0;
                break;
        }
        
        eventCb(extEvent, data, len, _aiInstance->getUserArg());
    }
}

static AI_CHAT_MODE_E _convertToInternalMode(AIChatMode_t mode) {
    switch (mode) {
        case AI_MODE_HOLD:    return AI_CHAT_MODE_HOLD;
        case AI_MODE_ONESHOT: return AI_CHAT_MODE_ONE_SHOT;
        case AI_MODE_WAKEUP:  return AI_CHAT_MODE_WAKEUP;
        case AI_MODE_FREE:    return AI_CHAT_MODE_FREE;
        default:              return AI_CHAT_MODE_WAKEUP;
    }
}

static AIChatMode_t _convertFromInternalMode(AI_CHAT_MODE_E mode) {
    switch (mode) {
        case AI_CHAT_MODE_HOLD:     return AI_MODE_HOLD;
        case AI_CHAT_MODE_ONE_SHOT: return AI_MODE_ONESHOT;
        case AI_CHAT_MODE_WAKEUP:   return AI_MODE_WAKEUP;
        case AI_CHAT_MODE_FREE:     return AI_MODE_FREE;
        default:                    return AI_MODE_WAKEUP;
    }
}

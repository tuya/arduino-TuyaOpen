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
#include "cJSON.h"
extern "C" {
#include "tuya_ai_agent.h"

}
#include "ai_chat_main.h"
#include "ai_agent.h"
#include "ai_manage_mode.h"
#include "ai_user_event.h"
#include "lang_config.h"

#if defined(ENABLE_COMP_AI_AUDIO) && (ENABLE_COMP_AI_AUDIO == 1)
#include "ai_audio_player.h"
#include "ai_audio_input.h"
#endif

#include "tdl_button_manage.h"
/***********************************************************
************************macro define************************
***********************************************************/
#define AI_CHAT_BUTTON_NAME    "ai_chat_button"
#define TUYA_AI_CHAT_PAR       "ty_ai_chat_par"

#define AI_AUDIO_SLICE_TIME         80     
#define AI_AUDIO_VAD_ACTIVE_TIME    200 
#define AI_AUDIO_VAD_OFF_TIME       1000
/***********************************************************
************************static variables********************
***********************************************************/
// Singleton instance
TuyaAIClass TuyaAI;
// Static callback storage for internal event routing
static TuyaAIClass *_aiInstance = nullptr;

static AI_USER_EVENT_NOTIFY   sg_evt_notify_cb = NULL;
static THREAD_HANDLE          sg_ai_chat_mode_task = NULL;
static AI_CHAT_MODE_E         sg_ai_default_mode = AI_CHAT_MODE_HOLD;
static int                    sg_ai_default_vol = 70;
static bool                   sg_ai_agent_inited = false;
/***********************************************************
***********************static function declarations*********
***********************************************************/
static void _internalEventHandler(AI_NOTIFY_EVENT_T *event);
static OPERATE_RET __ai_chat_save_config(uint32_t mode, int volume);
static OPERATE_RET __ai_chat_load_config(uint32_t *mode, int *volume);
static void __ai_chat_mode_task(void *args);
static void __ai_handle_event(AI_NOTIFY_EVENT_T *event);
static int __ai_mqtt_connected_evt(void *data);
static OPERATE_RET __ai_chat_mode_register(void);
static void ai_chat_ui_handle_event(AI_NOTIFY_EVENT_T *event);
/***********************************************************
***********************TuyaAIClass Implementation***********
***********************************************************/

TuyaAIClass::TuyaAIClass() {
    _initialized = false;
    _chatMode = AI_CHAT_MODE_WAKEUP;
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
    
    // Setup chat mode configuration
    AI_CHAT_MODE_CFG_T aiChatCfg = {
        .default_mode = _chatMode,
        .default_vol  = config.volume,
        .evt_cb       = _internalEventHandler,
    };
    
    // Initialize AI chat module
    uint32_t mode = sg_ai_default_mode;
    int vol = sg_ai_default_vol;

    TUYA_CALL_ERR_RETURN(__ai_chat_mode_register());

    sg_ai_default_mode = aiChatCfg.default_mode;
    sg_ai_default_vol  = aiChatCfg.default_vol;
    mode = sg_ai_default_mode;
    vol  = sg_ai_default_vol;

    rt = __ai_chat_load_config(&mode, &vol);
    if (OPRT_OK != rt) {
        mode = sg_ai_default_mode;
        vol = sg_ai_default_vol;
        TUYA_CALL_ERR_RETURN(__ai_chat_save_config(mode, vol));
        PR_ERR("load chat mode config failed, use default mode %d, volume %d", mode, vol);
    }

    sg_evt_notify_cb = aiChatCfg.evt_cb;

    ai_user_event_notify_register(__ai_handle_event);

    TUYA_CALL_ERR_RETURN(tal_event_subscribe(EVENT_MQTT_CONNECTED, "ai_agent_init", __ai_mqtt_connected_evt, SUBSCRIBE_TYPE_EMERGENCY));
    TUYA_CALL_ERR_RETURN(tal_event_subscribe(EVENT_AI_CLIENT_RUN, "client_run", ai_mode_client_run, SUBSCRIBE_TYPE_NORMAL));

    THREAD_CFG_T thrd_cfg = {
        .stackDepth = 3 * 1024,
        .priority = THREAD_PRIO_5,
        .thrdname = (char *)"ai_chat_mode",
#if defined(ENABLE_EXT_RAM) && (ENABLE_EXT_RAM == 1)
        .psram_mode = 1,
#endif
    };

    TUYA_CALL_ERR_RETURN(tal_thread_create_and_start(&sg_ai_chat_mode_task, NULL, NULL,\
                                                     __ai_chat_mode_task, NULL, &thrd_cfg));
    
    _initialized = true;
    return OPRT_OK;
}

OPERATE_RET TuyaAIClass::begin() {
    AIConfig_t config = {
        .chatMode = AI_CHAT_MODE_WAKEUP,
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

OPERATE_RET TuyaAIClass::setChatMode(AI_CHAT_MODE_E mode) {
    OPERATE_RET rt = ai_mode_switch(mode);
    if (rt == OPRT_OK) {
        _chatMode = mode;
    }
    return rt;
}

AI_CHAT_MODE_E TuyaAIClass::getChatMode() {
    AI_CHAT_MODE_E mode;
    ai_mode_get_curr_mode(&mode);
    return mode;
}

AI_CHAT_MODE_E TuyaAIClass::nextChatMode() {
    _chatMode = ai_mode_switch_next();
    return _chatMode;
}

AI_MODE_STATE_E TuyaAIClass::getState() {
    return ai_mode_get_state();
}

const char* TuyaAIClass::getStateString() {
    return ai_get_mode_state_str(ai_mode_get_state());
}

const char* TuyaAIClass::getModeString() {
    AI_CHAT_MODE_E internalMode;
    ai_mode_get_curr_mode(&internalMode);
    return ai_get_mode_name_str(internalMode);
}

OPERATE_RET TuyaAIClass::saveModeConfig(AI_CHAT_MODE_E mode, int volume) {
    return __ai_chat_save_config((uint32_t)mode, volume);
}

OPERATE_RET TuyaAIClass::loadModeConfig(AI_CHAT_MODE_E *mode, int *volume) {
    if (mode == nullptr || volume == nullptr) {
        return OPRT_INVALID_PARM;
    }
    uint32_t temp = 0;
    OPERATE_RET rt = __ai_chat_load_config(&temp, volume);
    *mode = (AI_CHAT_MODE_E)temp;
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

OPERATE_RET TuyaAIClass::requestCloudAlert(AI_AUDIO_ALERT_TYPE_E type) {
    return ai_agent_cloud_alert((AI_ALERT_TYPE_E)type);
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
    AI_USER_EVT_TYPE_E extEvent = (event->type);
    
    // Handle state changes
    if (event->type == AI_USER_EVT_MODE_STATE_UPDATE) {
        AIStateCallback_t stateCb = _aiInstance->getStateCallback();
        if (stateCb != nullptr) {
            AI_MODE_STATE_E state = _aiInstance->getState();
            stateCb(state);
        }
    }
    
    // Handle alert events
    if (event->type == AI_USER_EVT_PLAY_ALERT) {
        AIAlertCallback_t alertCb = _aiInstance->getAlertCallback();
        if (alertCb != nullptr) {
            // Let user handle alert first
            if (event->data) {
                AI_AUDIO_ALERT_TYPE_E alertType = (AI_AUDIO_ALERT_TYPE_E)(uintptr_t)event->data;
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

            case AI_USER_EVT_MODE_SWITCH:
                // Mode switch event - pass new mode as uint32_t
                if (event->data != nullptr) {
                    uint32_t *modePtr = (uint32_t *)event->data;
                    data = (uint8_t *)modePtr;
                    len = sizeof(uint32_t);
                }
                break;

            case AI_USER_EVT_MODE_STATE_UPDATE:
                // Mode state update event - pass new state as uint32_t
                break;
                
            default:
                data = (uint8_t *)event->data;
                len = 0;
                break;
        }
        
        eventCb(extEvent, data, len, _aiInstance->getUserArg());
    }
}


/**
@brief Save chat mode and volume configuration
@param mode Chat mode value
@param volume Volume value
@return OPERATE_RET Operation result
*/
static OPERATE_RET __ai_chat_save_config(uint32_t mode, int volume)
{
    OPERATE_RET rt = OPRT_OK;
    char buf[64] = {0};

    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf), "{\"volume\": %d, \"chat_mode\":%d}", volume, (int)mode);
    PR_DEBUG("save ai_toy config: %s", buf);
    TUYA_CALL_ERR_RETURN(tal_kv_set(TUYA_AI_CHAT_PAR, (const uint8_t *)buf, strlen(buf)));
    PR_DEBUG("save volume config: %s", buf);

    PR_DEBUG("save chat mode config: %s", buf);
    return rt;
}

/**
@brief Load chat mode and volume configuration
@param mode Pointer to store mode value
@param volume Pointer to store volume value
@return OPERATE_RET Operation result
*/
static OPERATE_RET __ai_chat_load_config(uint32_t *mode, int *volume)
{
    OPERATE_RET rt = OPRT_OK;
    uint8_t *value = NULL;
    size_t len = 0;
    uint32_t read_mode = 0;
    int read_vol = sg_ai_default_vol;

    if(NULL == mode || NULL == volume) {
        return OPRT_INVALID_PARM;
    }

    /* Read volume from KV */
    PR_DEBUG("load ai_toy config");
    TUYA_CALL_ERR_RETURN(tal_kv_get(TUYA_AI_CHAT_PAR, &value, &len));
    PR_DEBUG("read ai_toy config: %s", value);

    cJSON *root = cJSON_Parse((const char *)value);
    tal_kv_free(value);
    TUYA_CHECK_NULL_RETURN(root, OPRT_FILE_READ_FAILED);

    /* Read volume */
    cJSON *volum = cJSON_GetObjectItem(root, "volume");
    if (volum) {
        if (volum->valueint <= 100 && volum->valueint >= 0) {
            read_vol = volum->valueint;
        }
    }

    /* Read trigger mode */
    cJSON *chat_mode = cJSON_GetObjectItem(root, "chat_mode");
    if (chat_mode) {
        read_mode = (uint32_t)chat_mode->valueint;
    }

    cJSON_Delete(root);

    *mode = read_mode;
    *volume = read_vol;

    return OPRT_OK;
}

/**
@brief Handle AI user events
@param event Pointer to event structure
@return None
*/
static void __ai_handle_event(AI_NOTIFY_EVENT_T *event)
{
    if(NULL == event) {
        return;
    }

    ai_mode_handle_event(event);

    switch(event->type) {
#if defined(ENABLE_COMP_AI_AUDIO) && (ENABLE_COMP_AI_AUDIO == 1)
        case AI_USER_EVT_PLAY_CTL_PLAY:
        case AI_USER_EVT_PLAY_CTL_RESUME:{
            ai_audio_player_set_resume(true);
        }
        break;
        case AI_USER_EVT_PLAY_CTL_PAUSE:{
            ai_audio_player_stop(AI_AUDIO_PLAYER_BG);
        }
        break;
        case AI_USER_EVT_PLAY_CTL_REPLAY:{
            ai_audio_player_set_replay(true);
        }
        break;
        case AI_USER_EVT_PLAY_ALERT:{
            ai_audio_player_alert((AI_AUDIO_ALERT_TYPE_E)(uintptr_t)event->data);
        }
        break;
#endif
        default:
#if defined(ENABLE_COMP_AI_DISPLAY) && (ENABLE_COMP_AI_DISPLAY == 1)
            ai_chat_ui_handle_event(event);
#endif
        break;
    }

    if (sg_evt_notify_cb) {
        sg_evt_notify_cb(event);
    }   
}

static int __ai_mqtt_connected_evt(void *data)
{
    PR_DEBUG("AI MQTT connected event received");
    OPERATE_RET rt = OPRT_OK;
    uint32_t mode = sg_ai_default_mode;
    int vol = sg_ai_default_vol;

    TUYA_CALL_ERR_RETURN(ai_agent_init());

    TUYA_CALL_ERR_RETURN(__ai_chat_load_config(&mode, &vol));
    TUYA_CALL_ERR_RETURN(ai_mode_init((AI_CHAT_MODE_E)mode));

    sg_ai_agent_inited = true;
    PR_DEBUG("ai mqtt connected evt, init ai agent");

    return rt;
}

/**
@brief AI chat mode task function
@param args Task arguments
@return None
*/
static void __ai_chat_mode_task(void *args)
{
    while(tal_thread_get_state(sg_ai_chat_mode_task) == THREAD_STATE_RUNNING) {
        ai_mode_task_running(args);
        tal_system_sleep(20);
    }
}

static OPERATE_RET __ai_chat_mode_register(void)
{
    OPERATE_RET rt = OPRT_OK;

#if defined(ENABLE_COMP_AI_MODE_HOLD) && (ENABLE_COMP_AI_MODE_HOLD == 1)
    TUYA_CALL_ERR_RETURN(ai_mode_hold_register());
#endif

#if defined(ENABLE_COMP_AI_MODE_ONESHOT) && (ENABLE_COMP_AI_MODE_ONESHOT == 1)
    TUYA_CALL_ERR_RETURN(ai_mode_oneshot_register());
#endif

#if defined(ENABLE_COMP_AI_MODE_WAKEUP) && (ENABLE_COMP_AI_MODE_WAKEUP == 1)
    TUYA_CALL_ERR_RETURN(ai_mode_wakeup_register());
#endif

#if defined(ENABLE_COMP_AI_MODE_FREE) && (ENABLE_COMP_AI_MODE_FREE == 1)
    TUYA_CALL_ERR_RETURN(ai_mode_free_register());
#endif

    return rt;
}

static void __ai_chat_disp_mode_state(AI_MODE_STATE_E state)
{
    switch (state) {
    case AI_MODE_STATE_INIT:
    case AI_MODE_STATE_IDLE:
        ai_ui_disp_msg(AI_UI_DISP_EMOTION, (uint8_t *)EMOJI_NEUTRAL, strlen(EMOJI_NEUTRAL));
        ai_ui_disp_msg(AI_UI_DISP_STATUS, (uint8_t *)STANDBY, strlen(STANDBY));
        break;
    case AI_MODE_STATE_LISTEN:
        ai_ui_disp_msg(AI_UI_DISP_STATUS, (uint8_t *)LISTENING, strlen(LISTENING));
        break;
    case AI_MODE_STATE_SPEAK:
        ai_ui_disp_msg(AI_UI_DISP_STATUS, (uint8_t *)SPEAKING, strlen(SPEAKING));
        break;
    default:
        break;
    }
}

static void ai_chat_ui_handle_event(AI_NOTIFY_EVENT_T *event)
{
    AI_NOTIFY_TEXT_T *text = NULL;

    if (NULL == event) {
        return;
    }

    switch (event->type) {
        case AI_USER_EVT_ASR_OK: {
            text = (AI_NOTIFY_TEXT_T *)event->data;

            if (text && text->datalen > 0 && text->data) {
                ai_ui_disp_msg(AI_UI_DISP_USER_MSG, (uint8_t *)text->data, text->datalen);
            }
        } break;
        case AI_USER_EVT_TEXT_STREAM_START: {
            ai_ui_disp_msg(AI_UI_DISP_AI_MSG_STREAM_START, NULL, 0);

            text = (AI_NOTIFY_TEXT_T *)event->data;
            if (text && text->datalen > 0 && text->data) {
                ai_ui_disp_msg(AI_UI_DISP_AI_MSG_STREAM_DATA, (uint8_t *)text->data, text->datalen);
            }
        } break;
        case AI_USER_EVT_TEXT_STREAM_DATA: {
            text = (AI_NOTIFY_TEXT_T *)event->data;
            if (text && text->datalen > 0 && text->data) {
                ai_ui_disp_msg(AI_UI_DISP_AI_MSG_STREAM_DATA, (uint8_t *)text->data, text->datalen);
            }
        } break;
        case AI_USER_EVT_TEXT_STREAM_STOP: {
            text = (AI_NOTIFY_TEXT_T *)event->data;
            if (text && text->datalen > 0 && text->data) {
                ai_ui_disp_msg(AI_UI_DISP_AI_MSG_STREAM_DATA, (uint8_t *)text->data, text->datalen);
            }
            ai_ui_disp_msg(AI_UI_DISP_AI_MSG_STREAM_END, NULL, 0);
        } break;
        case AI_USER_EVT_CHAT_BREAK: {
            ai_ui_disp_msg(AI_UI_DISP_AI_MSG_STREAM_INTERRUPT, NULL, 0);
        } break;
        case AI_USER_EVT_LLM_EMOTION:
        case AI_USER_EVT_EMOTION: {
            AI_NOTIFY_EMO_T *emo = (AI_NOTIFY_EMO_T *)(event->data);

            if (emo) {
                PR_NOTICE("emoji: %s, name: %s", emo->emoji, emo->name);
                ai_ui_disp_msg(AI_UI_DISP_EMOTION, (uint8_t *)emo->name, strlen(emo->name));
            }
        } break;
        case AI_USER_EVT_MODE_STATE_UPDATE: {
            AI_MODE_STATE_E state = (AI_MODE_STATE_E)(uintptr_t)event->data;
            __ai_chat_disp_mode_state(state);
        } break;
        case AI_USER_EVT_MODE_SWITCH: {
            // AI_CHAT_MODE_E mode = (AI_CHAT_MODE_E)(uintptr_t)event->data;
            // char          *name = ai_get_mode_name_str(mode);
            // if (NULL == name) {
            //     PR_NOTICE("mode name str is null");
            //     break;
            // }
            if (event->data != nullptr) {
                int modeValue = (int)(intptr_t)event->data;
                PR_DEBUG("Internal mode switch event, new mode: %d", modeValue);
                const char *modeStr = NULL;
                switch (modeValue) {
                    case AI_CHAT_MODE_HOLD:     modeStr = HOLD_TALK; break;
                    case AI_CHAT_MODE_ONE_SHOT: modeStr = TRIG_TALK; break;
                    case AI_CHAT_MODE_WAKEUP:   modeStr = WAKEUP_TALK; break;
                    case AI_CHAT_MODE_FREE:     modeStr = FREE_TALK; break;
                    default:                    modeStr = "---"; break;
                }
                ai_ui_disp_msg(AI_UI_DISP_CHAT_MODE, (uint8_t *)modeStr, strlen(modeStr));
            }
        } break;
        case AI_USER_EVT_VIDEO_DISPLAY_START: {
            AI_NOTIFY_VIDEO_START_T *video_start = (AI_NOTIFY_VIDEO_START_T *)(event->data);
            if (NULL == video_start) {
                PR_ERR("video start param is null");
                break;
            }

            ai_ui_camera_start(video_start->camera_width, video_start->camera_height);
        } break;
        case AI_USER_EVT_VIDEO_DISPLAY_END:
            ai_ui_camera_end();
            break;
        default:
            break;
    }
}

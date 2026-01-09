/**
 * @file TuyaAI.cpp
 * @author Tuya Inc.
 * @brief AI audio component implementation
 *
 * This source file provides the implementation of the AI audio component,
 * including functions for playing alert sounds, handling AI events,
 * and managing audio input and output.
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 *
 */
#include "TuyaAI.h"

extern "C" {
#include "tkl_output.h"
#include "tal_api.h"
#if LANG_CODE_ZH
#include "src/media/media_src_zh.h"
#else
#include "src/media/media_src_en.h"
#endif
#include "lang_config.h"
}
/**
 * @brief Plays an alert sound based on the specified alert type.
 *
 * @param type - The type of alert to play, defined by the APP_ALERT_TYPE_E enum.
 * @return OPERATE_RET - Returns OPRT_OK if the alert sound is successfully played, otherwise returns an error code.
 */
static OPERATE_RET ai_audio_player_play_alert(AI_AUDIO_ALERT_TYPE_E type)
{
    OPERATE_RET rt = OPRT_OK;
    char alert_id[64] = {0};

    snprintf(alert_id, sizeof(alert_id), "alert_%d", type);

    rt = ai_audio_player_start(alert_id);
#if LANG_CODE_ZH
    switch (type) {
        case AI_AUDIO_ALERT_POWER_ON: {
            rt = ai_audio_player_data_write(alert_id, (uint8_t *)media_src_prologue_zh, sizeof(media_src_prologue_zh), 1);
        } break;
        case AI_AUDIO_ALERT_NOT_ACTIVE: {
            rt = ai_audio_player_data_write(alert_id, (uint8_t *)media_src_network_conn_zh,
                                            sizeof(media_src_network_conn_zh), 1);
        } break;
        case AI_AUDIO_ALERT_NETWORK_CFG: {
            rt = ai_audio_player_data_write(alert_id, (uint8_t *)media_src_network_config_zh,
                                            sizeof(media_src_network_config_zh), 1);
        } break;
        case AI_AUDIO_ALERT_NETWORK_CONNECTED: {
            rt = ai_audio_player_data_write(alert_id, (uint8_t *)media_src_network_conn_success_zh,
                                            sizeof(media_src_network_conn_success_zh), 1);
        } break;
        case AI_AUDIO_ALERT_NETWORK_FAIL: {
            rt = ai_audio_player_data_write(alert_id, (uint8_t *)media_src_network_conn_failed_zh,
                                            sizeof(media_src_network_conn_failed_zh), 1);
        } break;
        case AI_AUDIO_ALERT_NETWORK_DISCONNECT: {
            rt = ai_audio_player_data_write(alert_id, (uint8_t *)media_src_network_reconfigure_zh,
                                            sizeof(media_src_network_reconfigure_zh), 1);
        } break;
        case AI_AUDIO_ALERT_BATTERY_LOW: {
            rt = ai_audio_player_data_write(alert_id, (uint8_t *)media_src_low_battery_zh, sizeof(media_src_low_battery_zh),
                                            1);
        } break;
        case AI_AUDIO_ALERT_PLEASE_AGAIN: {
            rt = ai_audio_player_data_write(alert_id, (uint8_t *)media_src_please_again_zh,
                                            sizeof(media_src_please_again_zh), 1);
        } break;
        case AI_AUDIO_ALERT_WAKEUP: {
            rt = ai_audio_player_data_write(alert_id, (uint8_t *)media_src_ai_zh, sizeof(media_src_ai_zh), 1);
        } break;
        case AI_AUDIO_ALERT_LONG_KEY_TALK: {
            rt = ai_audio_player_data_write(alert_id, (uint8_t *)media_src_long_press_zh, sizeof(media_src_long_press_zh),
                                            1);
        } break;
        case AI_AUDIO_ALERT_KEY_TALK: {
            rt = ai_audio_player_data_write(alert_id, (uint8_t *)media_src_press_talk_zh, sizeof(media_src_press_talk_zh),
                                            1);
        } break;
        case AI_AUDIO_ALERT_WAKEUP_TALK: {
            rt = ai_audio_player_data_write(alert_id, (uint8_t *)media_src_wakeup_chat_zh, sizeof(media_src_wakeup_chat_zh),
                                            1);
        } break;
        case AI_AUDIO_ALERT_FREE_TALK: {
            rt = ai_audio_player_data_write(alert_id, (uint8_t *)media_src_free_chat_zh, sizeof(media_src_free_chat_zh), 1);
        } break;

        default:
            break;
        }
#else
    switch (type) {
        case AI_AUDIO_ALERT_POWER_ON: {
            rt = ai_audio_player_data_write(alert_id, (uint8_t *)media_src_prologue_en, sizeof(media_src_prologue_en), 1);
        } break;

        case AI_AUDIO_ALERT_NOT_ACTIVE: {
            rt = ai_audio_player_data_write(alert_id, (uint8_t *)media_src_network_conn_en,
                                            sizeof(media_src_network_conn_en), 1);
        } break;

        case AI_AUDIO_ALERT_NETWORK_CFG: {
            rt = ai_audio_player_data_write(alert_id, (uint8_t *)media_src_network_config_en,
                                            sizeof(media_src_network_config_en), 1);
        } break;

        case AI_AUDIO_ALERT_NETWORK_CONNECTED: {
            rt = ai_audio_player_data_write(alert_id, (uint8_t *)media_src_network_conn_success_en,
                                            sizeof(media_src_network_conn_success_en), 1);
        } break;

        case AI_AUDIO_ALERT_NETWORK_FAIL: {
            rt = ai_audio_player_data_write(alert_id, (uint8_t *)media_src_network_conn_failed_en,
                                            sizeof(media_src_network_conn_failed_en), 1);
        } break;

        case AI_AUDIO_ALERT_NETWORK_DISCONNECT: {
            rt = ai_audio_player_data_write(alert_id, (uint8_t *)media_src_network_reconfigure_en,
                                            sizeof(media_src_network_reconfigure_en), 1);
        } break;

        case AI_AUDIO_ALERT_BATTERY_LOW: {
            rt = ai_audio_player_data_write(alert_id, (uint8_t *)media_src_low_battery_en, sizeof(media_src_low_battery_en),
                                            1);
        } break;
        case AI_AUDIO_ALERT_PLEASE_AGAIN: {
            rt = ai_audio_player_data_write(alert_id, (uint8_t *)media_src_please_again_en,
                                            sizeof(media_src_please_again_en), 1);
        } break;

        case AI_AUDIO_ALERT_WAKEUP: {
            rt = ai_audio_player_data_write(alert_id, (uint8_t *)media_src_ai_en, sizeof(media_src_ai_en), 1);
        } break;

        case AI_AUDIO_ALERT_LONG_KEY_TALK: {
            rt = ai_audio_player_data_write(alert_id, (uint8_t *)media_src_long_press_en, sizeof(media_src_long_press_en),
                                            1);
        } break;

        case AI_AUDIO_ALERT_KEY_TALK: {
            rt = ai_audio_player_data_write(alert_id, (uint8_t *)media_src_press_talk_en, sizeof(media_src_press_talk_en),
                                            1);
        } break;

        case AI_AUDIO_ALERT_WAKEUP_TALK: {
            rt = ai_audio_player_data_write(alert_id, (uint8_t *)media_src_wakeup_chat_en, sizeof(media_src_wakeup_chat_en),
                                            1);
        } break;

        case AI_AUDIO_ALERT_FREE_TALK: {
            rt = ai_audio_player_data_write(alert_id, (uint8_t *)media_src_free_chat_en, sizeof(media_src_free_chat_en), 1);
        } break;

        default:
            break;
    }
#endif
    return rt;
}

typedef struct {
    APP_CHAT_MODE_E mode;
    AI_AUDIO_WORK_MODE_E auido_mode;
    AI_AUDIO_ALERT_TYPE_E mode_alert;
    const char *display_text;
    bool is_open;
} CHAT_WORK_MODE_INFO_T;

typedef struct {
    uint8_t is_enable;
    const CHAT_WORK_MODE_INFO_T *work;
} APP_CHAT_BOT_S;

// #if defined(ENABLE_CHAT_MODE_KEY_PRESS_HOLD_SINGEL) && (ENABLE_CHAT_MODE_KEY_PRESS_HOLD_SINGEL == 1)
const static CHAT_WORK_MODE_INFO_T cAPP_WORK_HOLD = {
    .mode = APP_CHAT_MODE_KEY_PRESS_HOLD_SINGLE,
    .auido_mode = AI_AUDIO_MODE_MANUAL_SINGLE_TALK,
    .mode_alert = AI_AUDIO_ALERT_LONG_KEY_TALK,
    .display_text = HOLD_TALK,
    .is_open = true,
};
// #elif defined(ENABLE_CHAT_MODE_KEY_TRIG_VAD_FREE) && (ENABLE_CHAT_MODE_KEY_TRIG_VAD_FREE == 1)
const static CHAT_WORK_MODE_INFO_T cAPP_WORK_TRIG_VAD = {
    .mode = APP_CHAT_MODE_KEY_TRIG_VAD_FREE,
    .auido_mode = AI_AUDIO_WORK_VAD_FREE_TALK,
    .mode_alert = AI_AUDIO_ALERT_KEY_TALK,
    .display_text = TRIG_TALK,
    .is_open = false,
};
// #elif defined(ENABLE_CHAT_MODE_ASR_WAKEUP_SINGEL) && (ENABLE_CHAT_MODE_ASR_WAKEUP_SINGEL == 1)
const static CHAT_WORK_MODE_INFO_T cAPP_WORK_WAKEUP_SINGLE = {
    .mode = APP_CHAT_MODE_ASR_WAKEUP_SINGLE,
    .auido_mode = AI_AUDIO_WORK_ASR_WAKEUP_SINGLE_TALK,
    .mode_alert = AI_AUDIO_ALERT_WAKEUP_TALK,
    .display_text = WAKEUP_TALK,
    .is_open = true,
};
// #elif defined(ENABLE_CHAT_MODE_ASR_WAKEUP_FREE) && (ENABLE_CHAT_MODE_ASR_WAKEUP_FREE == 1)
const static CHAT_WORK_MODE_INFO_T cAPP_WORK_WAKEUP_FREE = {
    .mode = APP_CHAT_MODE_ASR_WAKEUP_FREE,
    .auido_mode = AI_AUDIO_WORK_ASR_WAKEUP_FREE_TALK,
    .mode_alert = AI_AUDIO_ALERT_FREE_TALK,
    .display_text = FREE_TALK,
    .is_open = true,
};
// #endif


static APP_CHAT_BOT_S sg_chat_bot = {
    .is_enable = 0,
// #if defined(ENABLE_CHAT_MODE_KEY_PRESS_HOLD_SINGEL) && (ENABLE_CHAT_MODE_KEY_PRESS_HOLD_SINGEL == 1)
//     .work = &cAPP_WORK_HOLD,
// #elif defined(ENABLE_CHAT_MODE_KEY_TRIG_VAD_FREE) && (ENABLE_CHAT_MODE_KEY_TRIG_VAD_FREE == 1)
//     .work = &cAPP_WORK_TRIG_VAD,
// #elif defined(ENABLE_CHAT_MODE_ASR_WAKEUP_SINGEL) && (ENABLE_CHAT_MODE_ASR_WAKEUP_SINGEL == 1)
//     .work = &cAPP_WORK_WAKEUP_SINGLE,
// #elif defined(ENABLE_CHAT_MODE_ASR_WAKEUP_FREE) && (ENABLE_CHAT_MODE_ASR_WAKEUP_FREE == 1)
//     .work = &cAPP_WORK_WAKEUP_FREE,
// #else
    .work = &cAPP_WORK_HOLD,  // Default fallback
// #endif
};

static OPERATE_RET __app_chat_bot_enable(uint8_t enable)
{
    if (sg_chat_bot.is_enable == enable) {
        PR_DEBUG("chat bot enable is already %s", enable ? "enable" : "disable");
        return OPRT_OK;
    }

    PR_DEBUG("chat bot enable set %s", enable ? "enable" : "disable");

    OPERATE_RET rt = ai_audio_set_open(enable);
    if (rt != OPRT_OK) {
        PR_ERR("set open failed: %d", rt);
    }

    sg_chat_bot.is_enable = enable;

    return OPRT_OK;
}

TuyaAI::TuyaAI() 
{
}

TuyaAI::~TuyaAI() 
{
}

void TuyaAI::begin(AI_AUDIO_CONFIG_T ai_audio_cfg) 
{
    if (ai_audio_cfg.work_mode != 0) {
        switch (ai_audio_cfg.work_mode) {
            case AI_AUDIO_MODE_MANUAL_SINGLE_TALK:
            sg_chat_bot.work = &cAPP_WORK_HOLD;
            case AI_AUDIO_WORK_VAD_FREE_TALK:
            sg_chat_bot.work = &cAPP_WORK_TRIG_VAD;
            case AI_AUDIO_WORK_ASR_WAKEUP_SINGLE_TALK:
            sg_chat_bot.work = &cAPP_WORK_WAKEUP_SINGLE;
            case AI_AUDIO_WORK_ASR_WAKEUP_FREE_TALK:
            sg_chat_bot.work = &cAPP_WORK_WAKEUP_FREE;
                break;
            default:
            sg_chat_bot.work = &cAPP_WORK_HOLD;
                break;
        }
    } else {
        PR_ERR("Work mode is not set, use default : %d", sg_chat_bot.work->auido_mode);
    }
    PR_DEBUG("ai audio work mode: %d", ai_audio_cfg.work_mode);
    OPERATE_RET rt = ai_audio_init(&ai_audio_cfg);
    if (rt != OPRT_OK) {
        PR_ERR("AI audio init failed: %d", rt);
    }

    if (sg_chat_bot.work != NULL) {
        OPERATE_RET rt = __app_chat_bot_enable(sg_chat_bot.work->is_open);
        if (rt != OPRT_OK) {
            PR_ERR("chat bot enable failed: %d", rt);
        }
    }
}

void TuyaAI::end() 
{
    // Clean up AI components here
    OPERATE_RET rt = __app_chat_bot_enable(0);
    if (rt != OPRT_OK) {
        PR_ERR("chat bot disable failed: %d", rt);
    }
    return;
}

// Volume control
int TuyaAI::setVolume(uint8_t volume) 
{
    OPERATE_RET rt = ai_audio_set_volume(volume);
    if (rt != OPRT_OK) {
        PR_ERR("set volume failed: %d", rt);
    }
    return rt;
}

uint8_t TuyaAI::getVolume() 
{
    return ai_audio_get_volume();
}

uint8_t TuyaAI::getWorkMode() 
{
    return sg_chat_bot.work->mode;
}

// Audio control
int TuyaAI::setOpen(bool is_open) 
{
    OPERATE_RET rt = __app_chat_bot_enable(is_open);
    if (rt != OPRT_OK) {
        PR_ERR("set open failed: %d", rt);
    }
    return rt;
}

int TuyaAI::manualStartSingleTalk() 
{
    OPERATE_RET rt = ai_audio_manual_start_single_talk();
    if (rt != OPRT_OK) {
        PR_ERR("manual start single talk failed: %d", rt);
    }
    return rt;
}

int TuyaAI::manualStopSingleTalk() 
{
    OPERATE_RET rt = ai_audio_manual_stop_single_talk();
    if (rt != OPRT_OK) {
        PR_ERR("manual stop single talk failed: %d", rt);
    }
    return rt;
}

int TuyaAI::setWakeup() 
{
    OPERATE_RET rt = ai_audio_set_wakeup();
    if (rt != OPRT_OK) {
        PR_ERR("set wakeup failed: %d", rt);
    }
    return rt;
}

AI_AUDIO_STATE_E TuyaAI::getAudioState() 
{
    return ai_audio_get_state();
}

/* Player ctrl*/
int TuyaAI::playAlert(AI_AUDIO_ALERT_TYPE_E type) 
{
    OPERATE_RET rt = ai_audio_player_play_alert(type);
    if (rt != OPRT_OK) {
        PR_ERR("play alert %d failed: %d", type, rt);
    }
    return rt;
}

uint8_t TuyaAI::isPlaying()
{
    return ai_audio_player_is_playing();
}

int TuyaAI::stopPlaying() 
{
    OPERATE_RET rt = ai_audio_player_stop();
    if (rt != OPRT_OK) {
        PR_ERR("stop playing failed: %d", rt);
    }
    return rt;
}

int TuyaAI::playerDataWrite(char *id, uint8_t *data, uint32_t len, uint8_t is_eof) 
{
    OPERATE_RET rt = ai_audio_player_data_write(id, data, len, is_eof);
    if (rt != OPRT_OK) {
        PR_ERR("player data write failed: %d", rt);
    }
    return rt;
}

/* Agent ctrl*/
int TuyaAI::textInput(uint8_t* text, size_t len) 
{
    OPERATE_RET rt = ai_text_agent_upload(text, len);
    if (rt != OPRT_OK) {
        PR_ERR("text input failed: %d", rt);
    }
    return rt;
}

int TuyaAI::audioInput(uint8_t* data, size_t len) 
{
    OPERATE_RET rt = ai_audio_agent_upload_data(data, len);
    if (rt != OPRT_OK) {
        PR_ERR("audio input failed: %d", rt);
    }
    return rt;
}

int TuyaAI::uploadStart(uint8_t enable_vad) 
{
    OPERATE_RET rt = ai_audio_agent_upload_start(enable_vad);
    if (rt != OPRT_OK) {
        PR_ERR("upload start failed: %d", rt);
    }
    return rt;
}

int TuyaAI::uploadStop() 
{
    OPERATE_RET rt = ai_audio_agent_upload_stop();
    if (rt != OPRT_OK) {
        PR_ERR("upload stop failed: %d", rt);
    }
    return rt;
}

int TuyaAI::uploadInterrupt(uint8_t enable_vad) 
{
    OPERATE_RET rt = ai_audio_agent_chat_intrrupt();
    if (rt != OPRT_OK) {
        PR_ERR("upload interrupt failed: %d", rt);
    }
    return rt;
}

// ASR ctrl
int TuyaAI::asrStart() 
{
    OPERATE_RET rt = ai_audio_cloud_asr_start();
    if (rt != OPRT_OK) {
        PR_ERR("ASR start failed: %d", rt);
    }
    return rt;
}

int TuyaAI::asrStop() 
{
    OPERATE_RET rt = ai_audio_cloud_asr_stop();
    if (rt != OPRT_OK) {
        PR_ERR("ASR stop failed: %d", rt);
    }
    return rt;
}

int TuyaAI::getAsrState() 
{
    return ai_audio_cloud_asr_get_state();
}

/* Input ctrl*/
int TuyaAI::enableGetValidData(bool is_enable) 
{
    OPERATE_RET rt = ai_audio_input_enable_get_valid_data(is_enable);
    if (rt != OPRT_OK) {
        PR_ERR("ASR data write failed: %d", rt);
    }
    return rt;
}   

int TuyaAI::manualOpenGetValidData(bool is_open) 
{
    OPERATE_RET rt = ai_audio_input_manual_open_get_valid_data(is_open);
    if (rt != OPRT_OK) {
        PR_ERR("manual open get valid data failed: %d", rt);
    }
    return rt;
}

int TuyaAI::stopAsrAwake() 
{
    OPERATE_RET rt = ai_audio_input_stop_asr_awake();
    if (rt != OPRT_OK) {
        PR_ERR("stop ASR awake failed: %d", rt);
    }
    return rt;
}

int TuyaAI::restartAsrAwakeTimer() 
{
    OPERATE_RET rt = ai_audio_input_restart_asr_awake_timer();
    if (rt != OPRT_OK) {
        PR_ERR("restart ASR awake timer failed: %d", rt);
    }
    return rt;
}

uint32_t TuyaAI::getInputData(uint8_t* buff, uint32_t len) 
{
    return ai_audio_get_input_data(buff, len);
}

uint32_t TuyaAI::getInputDataSize() 
{
    return ai_audio_get_input_data_size();
}

void TuyaAI::discardInputData(uint32_t discard_size) 
{
    ai_audio_discard_input_data(discard_size);
}

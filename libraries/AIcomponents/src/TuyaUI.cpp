/**
 * @file TuyaUI.cpp
 * @author Tuya Inc.
 * @brief TuyaUIClass implementation - UI display management
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 */
#include "TuyaAI.h"
#include <string.h>

#if defined(ENABLE_COMP_AI_DISPLAY) && (ENABLE_COMP_AI_DISPLAY == 1)
#include "ai_ui_manage.h"
#include "ai_ui_chat_wechat.h"
// #include "ai_ui_chat_chatbot.h"
#include "ai_ui_chat_oled.h"
#include "ai_ui_icon_font.h"
#endif

#include "lv_port_disp.h"
#include "lv_vendor.h"

/***********************************************************
***********************TuyaUIClass Implementation***********
***********************************************************/

TuyaUIClass::TuyaUIClass() {
    _initialized = false;
    _uiType = BOT_UI_WECHAT;
    _displayCallback = nullptr;
}

TuyaUIClass::~TuyaUIClass() {
    if (_initialized) {
        end();
    }
}

OPERATE_RET TuyaUIClass::begin(int uiType) {
    if (_initialized) {
        return OPRT_OK;
    }
    
    _uiType = uiType;
    OPERATE_RET rt = OPRT_OK;

    switch (uiType) {
        case BOT_UI_USER:
            // User custom UI - Only init LVGL
            static char display_name[] = "display";
            lv_vendor_init(display_name);
            lv_vendor_start(5, 1024*8);
            break;

        case BOT_UI_WECHAT:
            TUYA_CALL_ERR_RETURN(ai_ui_chat_wechat_register());
            break;

        // case BOT_UI_CHATBOT:
            // TUYA_CALL_ERR_RETURN(ai_ui_chat_chatbot_register());
            // break;

        // case BOT_UI_OLED:
            // TUYA_CALL_ERR_RETURN(ai_ui_chat_oled_register());
            // break;

        default:
            TUYA_CALL_ERR_RETURN(ai_ui_chat_wechat_register());
            break;
    }
    
#if defined(ENABLE_COMP_AI_DISPLAY) && (ENABLE_COMP_AI_DISPLAY == 1)
    // Initialize built-in UI if not using user custom UI
    if (uiType != BOT_UI_USER) {
        // Internal UI initialization
        rt = ai_ui_init();
        if (rt != OPRT_OK) {
            return rt;
        }
    }
#endif
    
    _initialized = true;
    return OPRT_OK;
}

OPERATE_RET TuyaUIClass::begin(UIConfig_t &config) {
    _displayCallback = config.displayCb;
    return begin(config.uiType);
}

void TuyaUIClass::end() {
    if (!_initialized) {
        return;
    }
    
    // UI cleanup is handled by individual UI implementations
    
    _initialized = false;
}

bool TuyaUIClass::isInitialized() {
    return _initialized;
}

int TuyaUIClass::getType() {
    return _uiType;
}

OPERATE_RET TuyaUIClass::displayMessage(AI_UI_DISP_TYPE_E type, uint8_t *data, int len) {
    // If using custom UI callback, forward to it
    if (_uiType == BOT_UI_USER && _displayCallback != nullptr) {
        _displayCallback(type, data, len);
        return OPRT_OK;
    }
    
#if defined(ENABLE_COMP_AI_DISPLAY) && (ENABLE_COMP_AI_DISPLAY == 1)
    PR_DEBUG("Displaying message of type %d ,data %d, length %d", type, *data, len);
    return ai_ui_disp_msg(type, data, len);
#else
    return OPRT_NOT_SUPPORTED;
#endif
}

OPERATE_RET TuyaUIClass::displayText(const char *text, bool isUserMessage) {
    if (text == nullptr) return OPRT_INVALID_PARM;
    AI_UI_DISP_TYPE_E type = isUserMessage ? AI_UI_DISP_USER_MSG : AI_UI_DISP_AI_MSG;
    return displayMessage(type, (uint8_t *)text, strlen(text));
}

OPERATE_RET TuyaUIClass::displayEmotion(const char *emotion) {
    if (emotion == nullptr) return OPRT_INVALID_PARM;
    return displayMessage(AI_UI_DISP_EMOTION, (uint8_t *)emotion, strlen(emotion));
}

OPERATE_RET TuyaUIClass::displayNotification(const char *notification) {
    if (notification == nullptr) return OPRT_INVALID_PARM;
    return displayMessage(AI_UI_DISP_NOTIFICATION, (uint8_t *)notification, strlen(notification));
}

OPERATE_RET TuyaUIClass::displayWifiStatus(AI_UI_WIFI_STATUS_E status) {
    return displayMessage(AI_UI_DISP_NETWORK, (uint8_t *)&status, sizeof(status));
}

OPERATE_RET TuyaUIClass::displayChatMode(AI_CHAT_MODE_E mode) {
    const char *modeStr;
    switch (mode) {
        case AI_CHAT_MODE_HOLD:     modeStr = "HOLD"; break;
        case AI_CHAT_MODE_ONE_SHOT: modeStr = "ONESHOT"; break;
        case AI_CHAT_MODE_WAKEUP:   modeStr = "WAKEUP"; break;
        case AI_CHAT_MODE_FREE:     modeStr = "FREE"; break;
        default:                    modeStr = "UNKNOWN"; break;
    }
    return displayMessage(AI_UI_DISP_CHAT_MODE, (uint8_t *)modeStr, strlen(modeStr));
}

void TuyaUIClass::enableUpdateLVGL() {
    disp_enable_update(NULL);
}

void TuyaUIClass::disableUpdateLVGL() {
    disp_disable_update(NULL);
}

void TuyaUIClass::setDisplayCallback(UIDisplayCallback_t callback) {
    _displayCallback = callback;
}

#if defined(ENABLE_LIBLVGL) && (ENABLE_LIBLVGL == 1)

lv_font_t *TuyaUIClass::getTextFont() {
    return ai_ui_get_text_font();
}

lv_font_t *TuyaUIClass::getIconFont() {
    return ai_ui_get_icon_font();
}

lv_font_t *TuyaUIClass::getEmoFont() {
    return ai_ui_get_emo_font();
}

UI_EMOJI_LIST_t *TuyaUIClass::getEmoList() {
    return (UI_EMOJI_LIST_t *)ai_ui_get_emo_list();
}

char *TuyaUIClass::getWifiIcon(AI_UI_WIFI_STATUS_E status) {
    return ai_ui_get_wifi_icon(status);
}

#endif

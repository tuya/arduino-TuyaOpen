/**
 * @file appButton.cpp
 * @brief Button handling for AI ChatBot
 *
 * Implements button interaction exactly matching ai_chat_main.c:
 * - Double click: Stop audio, interrupt chat, switch mode, play alert
 * - Other events: Delegate to ai_mode_handle_key
 *
 * Reference: __ai_button_function_cb in ai_chat_main.c
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 */

#include "appButton.h"
#include "TuyaAI.h"
#include "Button.h"
#include "Log.h"

/***********************************************************
************************macro define************************
***********************************************************/
#define DEFAULT_BUTTON_PIN  12
#define APP_BUTTON_NAME     "chatbot_btn"

/***********************************************************
***********************variable define**********************
***********************************************************/
static Button gButton;
static int gButtonPin = DEFAULT_BUTTON_PIN;
static bool gButtonReady = false;

/***********************************************************
***********************static declarations******************
***********************************************************/
static void onButtonEvent(char *name, ButtonEvent_t event, void *arg);

/***********************************************************
***********************public functions*********************
***********************************************************/

OPERATE_RET appButtonInit(int pin)
{
    PR_NOTICE("=== appButtonInit called with pin %d ===", pin);
    if (gButtonReady) return OPRT_OK;
    
    gButtonPin = pin;
    
    PinConfig_t pinCfg = {(uint8_t)gButtonPin, TUYA_GPIO_LEVEL_LOW, TUYA_GPIO_PULLUP};
    OPERATE_RET ret = gButton.begin(APP_BUTTON_NAME, pinCfg);
    if (ret != OPRT_OK) {
        PR_ERR("Button init failed: %d", ret);
        return OPRT_COM_ERROR;
    }
    
    gButton.setEventCallback(BUTTON_EVENT_PRESS_DOWN, onButtonEvent);
    gButton.setEventCallback(BUTTON_EVENT_PRESS_UP, onButtonEvent);
    gButton.setEventCallback(BUTTON_EVENT_SINGLE_CLICK, onButtonEvent);
    gButton.setEventCallback(BUTTON_EVENT_DOUBLE_CLICK, onButtonEvent);
    gButton.setEventCallback(BUTTON_EVENT_LONG_PRESS_START, onButtonEvent);
    
    gButtonReady = true;
    PR_NOTICE("Button on pin %d", gButtonPin);
    return OPRT_OK;
}

bool appButtonIsReady(void) { return gButtonReady; }
int appButtonGetPin(void) { return gButtonPin; }

/***********************************************************
***********************static implementations***************
***********************************************************/

/**
 * @brief Button event handler
 * 
 * Exactly matches __ai_button_function_cb in ai_chat_main.c:
 * - Double click: stop audio, interrupt, switch mode, play AI_AUDIO_ALERT_LONG_KEY_TALK + mode
 * - Other events: delegate to ai_mode_handle_key (via TuyaAI)
 */
static void onButtonEvent(char *name, ButtonEvent_t event, void *arg)
{
    if (!TuyaAI.isInitialized()) {
        PR_WARN("TuyaAI not initialized");
        return;
    }
    
    PR_NOTICE("User Button event: %d", event);
    
    // Double click handling - exactly as in __ai_button_function_cb
    if (event == BUTTON_EVENT_DOUBLE_CLICK) {
        // Stop all audio playback
        TuyaAI.Audio.stop();
        
        // Interrupt AI chat (sends AI_EVENT_CHAT_BREAK)
        TuyaAI.interruptChat();
        
        // Switch to next mode
        AI_CHAT_MODE_E nextMode = TuyaAI.nextChatMode();
        PR_DEBUG("Switching to mode: %d", nextMode);
        
        uint8_t volume = TuyaAI.getVolume();
        TuyaAI.saveModeConfig(nextMode, volume);

        AI_AUDIO_ALERT_TYPE_E alert = (AI_AUDIO_ALERT_TYPE_E)(AI_AUDIO_ALERT_LONG_KEY_TALK + (int)nextMode);
        TuyaAI.Audio.playAlert(alert);
        TuyaAI.Audio.startRecording();
        
        PR_DEBUG("Mode switched to: %d, alert: %d", nextMode, alert);
        return;
    }

    switch (event) {
        case BUTTON_EVENT_PRESS_DOWN:
            break;
        case BUTTON_EVENT_PRESS_UP:
            break;
        case BUTTON_EVENT_SINGLE_CLICK:
            break;
        case BUTTON_EVENT_LONG_PRESS_START:
            break;
        default:
            break;
    }
    
    // Other events: delegate to ai_mode_handle_key via TuyaAI
    TuyaAI.modeKeyHandle((uint8_t)event, arg);
}

/**
 * @file 03_AI_AudioSave.ino
 * @brief AI Audio Chat with MIC PCM and TTS MP3 recording to SD card
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 * @note Only supports TUYA_T5AI platform
 * 
 * Features:
 * - Voice chat with AI
 * - Save MIC audio as PCM files (16-bit, 16kHz, mono)
 * - Save TTS audio as MP3 files
 * - Files saved to /ai_recordings on SD card
 * - Button controls for voice input and mode switching
 */

#include <Arduino.h>
#include "TuyaAI.h"
#include "TuyaIoT.h"
#include "Log.h"
#include "Button.h"
#include "appAudioRecord.h"
/***********************************************************
************************macro define************************
***********************************************************/
// Device credentials (replace with your own)
#define TUYA_DEVICE_UUID    "uuidxxxxxxxxxxxxxxxx"
#define TUYA_DEVICE_AUTHKEY "keyxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
#define TUYA_PRODUCT_ID     "9inb01mvjqh5zhhr"

// T5AI-Board Hardware pins
#define LED_PIN             1
#define BUTTON_PIN          12

#define DPID_VOLUME         3
#define APP_BUTTON_NAME     "audio_btn"

/***********************************************************
***********************variable define**********************
***********************************************************/
static Button gButton;
/***********************************************************
***********************static declarations******************
***********************************************************/
static void tuyaIoTEventCallback(tuya_event_msg_t *event);
static void aiEventCallback(AI_USER_EVT_TYPE_E event, uint8_t *data, uint32_t len, void *arg);
static void aiStateCallback(AI_MODE_STATE_E state);
static void onButtonEvent(char *name, ButtonEvent_t event, void *arg);
static void handleUserInput(void);

/***********************************************************
***********************public functions*********************
***********************************************************/
void setup()
{
    Serial.begin(115200);
    Log.begin();
    
    // Print startup banner
    PR_NOTICE("========== AI Audio Save ==========");
    PR_NOTICE("Compile time:        %s", __DATE__);
    PR_NOTICE("TuyaOpen version:    %s", OPEN_VERSION);
    PR_NOTICE("Platform chip:       %s", PLATFORM_CHIP);
    PR_NOTICE("Platform board:      %s", PLATFORM_BOARD);
    PR_NOTICE("===================================");

    board_register_hardware();
    
    // TuyaIoT setup
    TuyaIoT.resetNetcfg();
    TuyaIoT.setEventCallback(tuyaIoTEventCallback);
    TuyaIoT.setLicense(TUYA_DEVICE_UUID, TUYA_DEVICE_AUTHKEY);
    TuyaIoT.begin(TUYA_PRODUCT_ID, PROJECT_VERSION);

    // TuyaAI setup
    AIConfig_t aiConfig = {AI_CHAT_MODE_WAKEUP, 70, aiEventCallback, aiStateCallback, NULL};
    TuyaAI.begin(aiConfig);
    TuyaAI.Audio.begin();

    // Button setup
    PinConfig_t pinCfg = {(uint8_t)BUTTON_PIN, TUYA_GPIO_LEVEL_LOW, TUYA_GPIO_PULLUP};
    gButton.begin(APP_BUTTON_NAME, pinCfg);
    gButton.setEventCallback(BUTTON_EVENT_PRESS_DOWN, onButtonEvent);
    gButton.setEventCallback(BUTTON_EVENT_PRESS_UP, onButtonEvent);
    gButton.setEventCallback(BUTTON_EVENT_SINGLE_CLICK, onButtonEvent);
    gButton.setEventCallback(BUTTON_EVENT_DOUBLE_CLICK, onButtonEvent);
    gButton.setEventCallback(BUTTON_EVENT_LONG_PRESS_START, onButtonEvent);

    appAudioRecordInit();
    
    delay(2000);
    TuyaIoT.resetNetconfigCheck();
}

void loop()
{
    handleUserInput();
    // Flush buffered audio data to SD card
    appAudioRecordFlush();
    
    delay(10);
}

/***********************************************************
***********************static implementations***************
***********************************************************/

/***********************************************************
***********************AI Callbacks*************************
***********************************************************/

/**
 * @brief AI event callback - handles audio recording
 */
static void aiEventCallback(AI_USER_EVT_TYPE_E event, uint8_t *data, uint32_t len, void *arg)
{
    switch (event) {
        case AI_USER_EVT_ASR_OK:
            Serial.print("\n[ASR]: ");
            if (data && len > 0) {
                Serial.write(data, len);
            }
            Serial.println();
            break;
            
        case AI_USER_EVT_MIC_DATA:
            // Save MIC PCM data to file
            if (data && len > 0) {
                appMicRecordWrite((const uint8_t*)data, len);
            }
            break;
            
        case AI_USER_EVT_TTS_START:
            // TTS playback started - start recording
            Serial.println("[TTS] Start playing");
            appTtsRecordStart();
            break;
            
        case AI_USER_EVT_TTS_DATA:
            // Save TTS audio data (MP3 format from cloud)
            if (data != nullptr && len > 0) {
                PR_NOTICE("[TTS] Record: %d bytes", len);
                appTtsRecordWrite((const uint8_t*)data, len);
            }
            break;
            
        case AI_USER_EVT_TTS_STOP:
            // TTS playback stopped normally
            Serial.println("[TTS] Stop");
            appTtsRecordStop();
            break;
            
        case AI_USER_EVT_TTS_ABORT:
            // TTS playback aborted (interrupted)
            Serial.println("[TTS] Aborted");
            appTtsRecordStop();
            break;
            
        case AI_USER_EVT_TTS_ERROR:
            // TTS playback error
            Serial.println("[TTS] Error");
            appTtsRecordStop();
            break;
            
        case AI_USER_EVT_TEXT_STREAM_START:
            Serial.print("\n[AI]: ");
            if (data && len > 0) Serial.write(data, len);
            break;
            
        case AI_USER_EVT_TEXT_STREAM_DATA:
            if (data && len > 0) Serial.write(data, len);
            break;
            
        case AI_USER_EVT_TEXT_STREAM_STOP:
            Serial.println();
            break;
            
        default:
            break;
    }
}

/**
 * @brief AI state change callback - handles MIC recording state
 */
static void aiStateCallback(AI_MODE_STATE_E state)
{
    const char *stateStr;
    switch (state) {
        case AI_MODE_STATE_IDLE:   stateStr = "Idle"; break;
        case AI_MODE_STATE_INIT:   stateStr = "Init"; break;
        case AI_MODE_STATE_LISTEN: stateStr = "Listening"; break;
        case AI_MODE_STATE_UPLOAD: stateStr = "Uploading"; break;
        case AI_MODE_STATE_THINK:  stateStr = "Thinking"; break;
        case AI_MODE_STATE_SPEAK:  stateStr = "Speaking"; break;
        default:                   stateStr = "Unknown"; break;
    }
    Serial.print("[State] "); Serial.println(stateStr);
    
    // Start MIC recording when entering LISTENING state
    if (state == AI_MODE_STATE_LISTEN) {
        appMicRecordStart();
    }
    // Stop MIC recording when leaving LISTENING state
    else if (state == AI_MODE_STATE_UPLOAD || state == AI_MODE_STATE_THINK || 
             state == AI_MODE_STATE_IDLE || state == AI_MODE_STATE_INIT) {
        appMicRecordStop();
    }
}

static OPERATE_RET aiAudioVolumUpload(void)
{
    uint8_t volume = TuyaAI.getVolume();
    PR_DEBUG("DP upload volume:%d", volume);
    return TuyaIoT.write(DPID_VOLUME, &volume, 1);
}

static OPERATE_RET audioDpObjProc(dp_obj_recv_t *dpobj)
{
    for (uint32_t index = 0; index < dpobj->dpscnt; index++) {
        dp_obj_t *dp = dpobj->dps + index;
        PR_DEBUG("idx:%d dpid:%d type:%d", index, dp->id, dp->type);

        switch (dp->id) {
            case DPID_VOLUME: {
                uint8_t volume = dp->value.dp_value;
                PR_DEBUG("volume:%d", volume);
                TuyaAI.setVolume(volume);
                break;
            }
            default:
                break;
        }
    }
    return OPRT_OK;
}

static void tuyaIoTEventCallback(tuya_event_msg_t *event)
{
    switch (event->id) {
        case TUYA_EVENT_BIND_START:
            PR_INFO("Device Bind Start!");
            TuyaAI.Audio.playAlert(AI_AUDIO_ALERT_NETWORK_CFG);
            break;
            
        case TUYA_EVENT_MQTT_CONNECTED:
            PR_INFO("Device MQTT Connected!");
            tal_event_publish(EVENT_MQTT_CONNECTED, NULL);
            {
                static bool gFirstConnect = true;
                if (gFirstConnect) {
                    gFirstConnect = false;
                    aiAudioVolumUpload();
                }
            }
            break;
            
        case TUYA_EVENT_MQTT_DISCONNECT:
            PR_INFO("Device MQTT Disconnected!");
            tal_event_publish(EVENT_MQTT_DISCONNECTED, NULL);
            break;
            
        case TUYA_EVENT_TIMESTAMP_SYNC:
            PR_INFO("Sync timestamp:%d", event->value.asInteger);
            tal_time_set_posix(event->value.asInteger, 1);
            break;
            
        case TUYA_EVENT_RESET:
            PR_INFO("Device Reset:%d", event->value.asInteger);
            break;
            
        case TUYA_EVENT_DP_RECEIVE_OBJ: {
            dp_obj_recv_t *dpobj = event->value.dpobj;
            PR_DEBUG("SOC Rev DP Cmd t1:%d t2:%d CNT:%u", dpobj->cmd_tp, dpobj->dtt_tp, dpobj->dpscnt);
            if (dpobj->devid != NULL) {
                PR_DEBUG("devid.%s", dpobj->devid);
            }
            audioDpObjProc(dpobj);
            TuyaIoT.write((dpobj->dps->id), (dpobj->dps->value), 0);
        }
            break;
            
        case TUYA_EVENT_DP_RECEIVE_RAW: {
            dp_raw_recv_t *dpraw = event->value.dpraw;
            PR_DEBUG("SOC Rev DP Cmd t1:%d t2:%d", dpraw->cmd_tp, dpraw->dtt_tp);
            if (dpraw->devid != NULL) {
                PR_DEBUG("devid.%s", dpraw->devid);
            }
            uint32_t index = 0;
            dp_raw_t *dp = &dpraw->dp;
            PR_DEBUG("dpid:%d type:RAW len:%d data:", dp->id, dp->len);
            for (index = 0; index < dp->len; index++) {
                PR_DEBUG_RAW("%02x", dp->data[index]);
            }
            TuyaIoT.write((dpraw->dp.id), (dpraw->dp.data), (dpraw->dp.len), 3);
        }
            break;
            
        default:
            break;
    }
}

static void handleUserInput(void)
{
    static int i = 0;
    static uint8_t _recv_buf[256];
    while (Serial.available()) {
        char c = Serial.read();
        _recv_buf[i++] = c;
        if (c == '\n' || c == '\r') {
            TuyaAI.sendText(_recv_buf, i);
            Serial.print("\n[User]: ");
            Serial.write(_recv_buf, i);
            i = 0;
            memset(_recv_buf, 0, sizeof(_recv_buf));
        }
    }
}

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
        
        PR_DEBUG("Mode switched to: %d, alert: %d", nextMode, alert);
        return;
    }
    // Other events: delegate to ai_mode_handle_key via TuyaAI
    TuyaAI.modeKeyHandle((uint8_t)event, arg);
}
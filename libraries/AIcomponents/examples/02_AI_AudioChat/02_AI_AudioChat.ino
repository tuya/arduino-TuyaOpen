/**
 * @file AI_AudioChat.ino
 * @brief Main Arduino sketch for AI Audio Chat using TuyaAI
 *
 * This example demonstrates how to use the TuyaAI library with
 * nested classes for a complete AI Audio Chat implementation.
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 *
 * @note Only supports TUYA_T5AI platform
 */

#include <Arduino.h>
#include "TuyaAI.h"
#include "TuyaIoT.h"
#include "Log.h"
#include "netmgr.h"
#include "Button.h"
/***********************************************************
************************macro define************************
***********************************************************/
// Device credentials (replace with your own)
#define TUYA_DEVICE_UUID    "uuidxxxxxxxxxxxxxxxx"
#define TUYA_DEVICE_AUTHKEY "keyxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
#define TUYA_PRODUCT_ID     "9inb01mvjqh5zhhr"

#define LED_PIN             9
#define BUTTON_PIN          29

#define DPID_VOLUME         3
#define APP_BUTTON_NAME     "chatbot_btn"
/***********************************************************
***********************static declarations******************
***********************************************************/
static void tuyaIoTEventCallback(tuya_event_msg_t *event);
static void aiEventCallback(AI_USER_EVT_TYPE_E event, uint8_t *data, uint32_t len, void *arg);
static void aiStateCallback(AI_MODE_STATE_E state);
static void onButtonEvent(char *name, ButtonEvent_t event, void *arg);
static void handleUserInput();
/***********************************************************
***********************variable define**********************
***********************************************************/
static Button gButton;
/***********************************************************
***********************public functions*********************
***********************************************************/

void setup()
{
    // Initialize Serial and logging
    Serial.begin(115200);
    Log.begin();
    // Log.setLevel(LogClass::WARN);
    
    // Print startup banner
    PR_NOTICE("============ Tuya AI Audio Chat ==============");
    PR_NOTICE("Compile time:        %s", __DATE__);

    // Hardware initialization
    board_register_hardware();
    
    // Setup TuyaIoT
    TuyaIoT.resetNetcfg();  // Reset network config on 3 quick rst button presses
    TuyaIoT.setEventCallback(tuyaIoTEventCallback);
    TuyaIoT.setLicense(TUYA_DEVICE_UUID, TUYA_DEVICE_AUTHKEY);
    TuyaIoT.begin(TUYA_PRODUCT_ID, PROJECT_VERSION);

    // Initialize TuyaAI core
    AIConfig_t aiConfig = {AI_CHAT_MODE_WAKEUP, 70, aiEventCallback, aiStateCallback, nullptr};
    TuyaAI.begin(aiConfig);
    TuyaAI.Audio.begin();

    PinConfig_t pinCfg = {(uint8_t)BUTTON_PIN, TUYA_GPIO_LEVEL_LOW, TUYA_GPIO_PULLUP};
    if (OPRT_OK != gButton.begin(APP_BUTTON_NAME, pinCfg)) {
        PR_ERR("Button init failed");
    }
    gButton.setEventCallback(BUTTON_EVENT_PRESS_DOWN, onButtonEvent);
    gButton.setEventCallback(BUTTON_EVENT_PRESS_UP, onButtonEvent);
    gButton.setEventCallback(BUTTON_EVENT_SINGLE_CLICK, onButtonEvent);
    gButton.setEventCallback(BUTTON_EVENT_DOUBLE_CLICK, onButtonEvent);
    gButton.setEventCallback(BUTTON_EVENT_LONG_PRESS_START, onButtonEvent);
    PR_NOTICE("Setup complete, waiting for network...");
    
    // Wait for IoT initialization
    delay(2000);
    TuyaIoT.resetNetconfigCheck();
}

void loop()
{
    handleUserInput();
    delay(10);
}

/***********************************************************
***********************static implementations***************
***********************************************************/

/**
 * @brief AI event callback
 * Handle AI events like ASR results, TTS, emotions, etc.
 */
static void aiEventCallback(AI_USER_EVT_TYPE_E event, uint8_t *data, uint32_t len, void *arg)
{
    // PR_NOTICE("AI Event: %d", event);
    switch (event) {
        case AI_USER_EVT_ASR_OK:
            // ASR recognition successful - Cloud ASR result
            Serial.print("\n[EVT] AI_USER_EVT_ASR_OK");
            if (data != nullptr && len > 0) {
                Serial.print(" - User said: ");
                Serial.write(data, len);
            }
            break;
        case AI_USER_EVT_TEXT_STREAM_START:
            // AI text response stream started (includes MCP results)
            Serial.print("\n[EVT] AI_USER_EVT_TEXT_STREAM_START\n[AI Response]: ");
            if (data != nullptr && len > 0) {
                Serial.write(data, len);
            }
            break;
        case AI_USER_EVT_TEXT_STREAM_DATA:
            // AI text response streaming data chunk
            Serial.print("\n[AI Response]: ");
            if (data != nullptr && len > 0) {
                Serial.write(data, len);
            }
            break;
        default:
            break;
    }
}

/**
 * @brief AI state change callback
 * Handle state transitions and update display
 */
static void aiStateCallback(AI_MODE_STATE_E state)
{   
    PR_DEBUG("AI State: %d", state);
}

OPERATE_RET aiAudioVolumUpload(void) {
  uint8_t volume = TuyaAI.getVolume();
  PR_DEBUG("DP upload volume:%d", volume);
  return TuyaIoT.write(DPID_VOLUME, &volume, 1);
}

OPERATE_RET audioDpObjProc(dp_obj_recv_t *dpobj) {
  uint32_t index = 0;
  for (index = 0; index < dpobj->dpscnt; index++) {
    dp_obj_t *dp = dpobj->dps + index;
    PR_DEBUG("idx:%d dpid:%d type:%d ts:%u", index, dp->id, dp->type, dp->time_stamp);

    switch (dp->id) {
      case DPID_VOLUME:
        {
          uint8_t volume = dp->value.dp_value;
          PR_DEBUG("volume:%d", volume);
          TuyaAI.setVolume(volume);
          char volume_str[20] = { 0 };
          snprintf(volume_str, sizeof(volume_str), "%s%d", VOLUME, volume);
          break;
        }
      default:
        break;
    }
  }

  return OPRT_OK;
}
/**
 * @brief TuyaIoT event callback
 * Handle IoT platform events
 */
static void tuyaIoTEventCallback(tuya_event_msg_t *event)
{ 
    switch (event->id) {
        case TUYA_EVENT_BIND_START:
            TuyaAI.Audio.playAlert(AI_AUDIO_ALERT_NETWORK_CFG);
            break;
            
        case TUYA_EVENT_MQTT_CONNECTED:
            PR_INFO("Device MQTT Connected!");
            tal_event_publish(EVENT_MQTT_CONNECTED, NULL);
            static bool gFirstConnect = true;
            if (gFirstConnect) {
                gFirstConnect = false;
                aiAudioVolumUpload();
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

static void handleUserInput() {
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
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
#include "lang_config.h"

#include "cJSON.h"
#include "netmgr.h"

#include "Button.h"

/***********************************************************
************************macro define************************
***********************************************************/
// Device credentials (replace with your own)
#define TUYA_DEVICE_UUID    "uuidxxxxxxxxxxxxxxxx"
#define TUYA_DEVICE_AUTHKEY "keyxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"

#define TUYA_PRODUCT_ID     "9inb01mvjqh5zhhr"

// Hardware pins
// T5AI board Hardware pin
// #define LED_PIN             1
// #define BUTTON_PIN          12
// #define SPK_PIN             28

// T5AI-core Hardware pin
#define LED_PIN             9
#define BUTTON_PIN          29
#define SPK_PIN             39

#define DPID_VOLUME         3
#define APP_BUTTON_NAME     "chatbot_btn"
/***********************************************************
***********************static declarations******************
***********************************************************/
static void tuyaIoTEventCallback(tuya_event_msg_t *event);
static void aiEventCallback(AIEvent_t event, uint8_t *data, uint32_t len, void *arg);
static void aiStateCallback(AIState_t state);
static void handleUserInput();
static void onButtonEvent(char *name, ButtonEvent_t event, void *arg);
/***********************************************************
***********************variable define**********************
***********************************************************/
static Button gButton;
static uint8_t gNeedReset = 0;
/***********************************************************
***********************public functions*********************
***********************************************************/

void setup()
{
    // Initialize Serial and logging
    Serial.begin(115200);
    Log.begin();
    
    // Print startup banner
    PR_NOTICE("============ Tuya AI Audio Chat ==============");
    PR_NOTICE("Compile time:        %s", __DATE__);
    PR_NOTICE("TuyaOpen version:    %s", OPEN_VERSION);
    PR_NOTICE("Platform chip:       %s", PLATFORM_CHIP);
    PR_NOTICE("Platform board:      %s", PLATFORM_BOARD);
    PR_NOTICE("========================================");

    // Hardware initialization
    if (OPRT_OK != board_register_hardware()) {
        PR_ERR("Board hardware registration failed");
    }
    
    // Initialize LED pin
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    
    // Setup TuyaIoT
    TuyaIoT.resetNetcfg();  // Reset network config on 3 quick rst button presses
    TuyaIoT.setEventCallback(tuyaIoTEventCallback);
    TuyaIoT.setLicense(TUYA_DEVICE_UUID, TUYA_DEVICE_AUTHKEY);
    TuyaIoT.begin(TUYA_PRODUCT_ID, PROJECT_VERSION);

    // Initialize TuyaAI core
    AIConfig_t aiConfig = {AI_MODE_WAKEUP, 70, aiEventCallback, aiStateCallback, NULL};
    if (OPRT_OK != TuyaAI.begin(aiConfig)) {
        PR_ERR("TuyaAI initialization failed");
    }

    // Initialize Audio
    if (OPRT_OK != TuyaAI.Audio.begin(SPK_PIN)) {
        PR_ERR("TuyaAI Audio initialization failed");
    }

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
    // Main loop - most work is done in callbacks
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
static void aiEventCallback(AIEvent_t event, uint8_t *data, uint32_t len, void *arg)
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
        case AI_USER_EVT_MIC_DATA:
            break;

        case AI_USER_EVT_TTS_START:
            break;
        case AI_USER_EVT_TTS_DATA:
            break;
        case AI_USER_EVT_TTS_STOP:
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
        case AI_USER_EVT_TEXT_STREAM_STOP:
            break;
        default:
            break;
    }
}

/**
 * @brief AI state change callback
 * Handle state transitions and update display
 */
static void aiStateCallback(AIState_t state)
{
    const char *stateStr;
    switch (state) {
        case AI_STATE_IDLE:      stateStr = "AI IDLE"; break;
        case AI_STATE_STANDBY:   stateStr = STANDBY; break;
        case AI_STATE_LISTENING: stateStr = LISTENING; break;
        case AI_STATE_UPLOADING: stateStr = "Uploading"; break;
        case AI_STATE_THINKING:  stateStr = "Thinking"; break;
        case AI_STATE_SPEAKING:  stateStr = SPEAKING; break;
        default:                 stateStr = "AI IDLE"; break;
    }
    
    PR_DEBUG("AI State: %s", stateStr);
}
void userUpgradeNotify(tuya_iot_client_t *client, cJSON *upgrade) {
  PR_INFO("----- Upgrade information -----");
  PR_INFO("OTA Channel: %d", cJSON_GetObjectItem(upgrade, "type")->valueint);
  PR_INFO("Version: %s", cJSON_GetObjectItem(upgrade, "version")->valuestring);
  PR_INFO("Size: %s", cJSON_GetObjectItem(upgrade, "size")->valuestring);
  PR_INFO("MD5: %s", cJSON_GetObjectItem(upgrade, "md5")->valuestring);
  PR_INFO("HMAC: %s", cJSON_GetObjectItem(upgrade, "hmac")->valuestring);
  PR_INFO("URL: %s", cJSON_GetObjectItem(upgrade, "url")->valuestring);
  PR_INFO("HTTPS URL: %s", cJSON_GetObjectItem(upgrade, "httpsUrl")->valuestring);
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
    PR_DEBUG("Tuya Event ID:%d(%s)", event->id, EVENT_ID2STR(event->id));
    PR_INFO("Device Free heap %d", tal_system_get_free_heap_size());
    
    switch (event->id) {
        case TUYA_EVENT_BIND_START:
            PR_INFO("Device Bind Start!");
            if (gNeedReset == 1) {
                PR_INFO("Device Reset!");
                tal_system_reset();
            }
            TuyaAI.Audio.playAlert(AI_ALERT_NETWORK_CFG);
            break;

        case TUYA_EVENT_DIRECT_MQTT_CONNECTED:
            break;

        case TUYA_EVENT_BIND_TOKEN_ON:
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

            /* _recv_buf upgrade request */
        case TUYA_EVENT_UPGRADE_NOTIFY:
            PR_DEBUG("TUYA_EVENT_UPGRADE_NOTIFY");
            userUpgradeNotify(tuya_iot_client_get(), event->value.asJSON);
        break;

            
        case TUYA_EVENT_TIMESTAMP_SYNC:
            PR_INFO("Sync timestamp:%d", event->value.asInteger);
            tal_time_set_posix(event->value.asInteger, 1);
            break;
            
        case TUYA_EVENT_RESET:
            PR_INFO("Device Reset:%d", event->value.asInteger);
            gNeedReset = 1;
            break;
            
        case TUYA_EVENT_DP_RECEIVE_OBJ:
        {
            dp_obj_recv_t *dpobj = event->value.dpobj;
            PR_DEBUG("SOC Rev DP Cmd t1:%d t2:%d CNT:%u", dpobj->cmd_tp, dpobj->dtt_tp, dpobj->dpscnt);
            if (dpobj->devid != NULL) {
                PR_DEBUG("devid.%s", dpobj->devid);
            }

            audioDpObjProc(dpobj);
            TuyaIoT.write((dpobj->dps->id), (dpobj->dps->value), 0);
        }
            break;
            
        case TUYA_EVENT_DP_RECEIVE_RAW:
        {
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
            TuyaAI.snedText(_recv_buf, i);
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
        AIChatMode_t nextMode = TuyaAI.nextChatMode();
        PR_DEBUG("Switching to mode: %d", nextMode);
        
        uint8_t volume = TuyaAI.getVolume();
        TuyaAI.saveModeConfig(nextMode, volume);

        AIAlertType_t alert = (AIAlertType_t)(AI_ALERT_HOLD_TALK + (int)nextMode);
        TuyaAI.Audio.playAlert(alert);
        
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
/**
 * @file AI_Skill.ino
 * @brief AI Skill - handle music, emotion, and TTS events
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 * @note Only supports TUYA_T5AI platform
 */

#include <Arduino.h>
#include "TuyaAI.h"
#include "TuyaIoT.h"
#include "Log.h"
#include "Button.h"

#include "skill_handler.h"
/***********************************************************
************************macro define************************
***********************************************************/
// Device credentials
#define TUYA_DEVICE_UUID    "uuidxxxxxxxxxxxxxxxx"
#define TUYA_DEVICE_AUTHKEY "keyxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
#define TUYA_PRODUCT_ID     "9inb01mvjqh5zhhr"

#define LED_PIN             1
#define BUTTON_PIN          12

#define DPID_VOLUME         3
/***********************************************************
***********************variable define**********************
***********************************************************/
static Button gButton;
/***********************************************************
***********************static declarations******************
***********************************************************/
static void aiEventCallback(AI_USER_EVT_TYPE_E event, uint8_t *data, uint32_t len, void *arg);
static void tuyaIoTEventCallback(tuya_event_msg_t *event);
static void onButtonEvent(char *name, ButtonEvent_t event, void *arg);
static void handleUserInput();
/***********************************************************
***********************public functions*********************
***********************************************************/
void setup()
{
    Serial.begin(115200);
    Log.begin();
    Log.setLevel(LogClass::WARN);
    
    // Print startup banner
    PR_NOTICE("============ Tuya AI Skill ==============");
    PR_NOTICE("Compile time:        %s", __DATE__);

    board_register_hardware();
    pinMode(LED_PIN, OUTPUT);
    
    // TuyaIoT setup
    TuyaIoT.resetNetcfg();
    TuyaIoT.setEventCallback(tuyaIoTEventCallback);
    TuyaIoT.setLicense(TUYA_DEVICE_UUID, TUYA_DEVICE_AUTHKEY);
    TuyaIoT.begin(TUYA_PRODUCT_ID, PROJECT_VERSION);

    // TuyaAI setup
    AIConfig_t cfg = {AI_CHAT_MODE_WAKEUP, 70, aiEventCallback, NULL, NULL};
    TuyaAI.begin(cfg);
    TuyaAI.Audio.begin();
    
    // Initialize skill handler
    skillInit();

    // Button setup
    PinConfig_t pinCfg = {(uint8_t)BUTTON_PIN, TUYA_GPIO_LEVEL_LOW, TUYA_GPIO_PULLUP};
    gButton.begin("btn", pinCfg);
    gButton.setEventCallback(BUTTON_EVENT_PRESS_DOWN, onButtonEvent);
    gButton.setEventCallback(BUTTON_EVENT_PRESS_UP, onButtonEvent);
    gButton.setEventCallback(BUTTON_EVENT_SINGLE_CLICK, onButtonEvent);
    gButton.setEventCallback(BUTTON_EVENT_DOUBLE_CLICK, onButtonEvent);
    gButton.setEventCallback(BUTTON_EVENT_LONG_PRESS_START, onButtonEvent);

    delay(2000);
    TuyaIoT.resetNetconfigCheck();
}

void loop()
{
    handleUserInput();
    delay(10);
}

/***********************************************************
***********************Callbacks****************************
***********************************************************/
static void aiEventCallback(AI_USER_EVT_TYPE_E event, uint8_t *data, uint32_t len, void *arg)
{
    switch (event) {
        case AI_USER_EVT_ASR_OK:
            if (data && len > 0) {
                Serial.print("[ASR]: ");
                Serial.write(data, len);
                Serial.println();
            }
            break;
        case AI_USER_EVT_TTS_START:
            break;
        case AI_USER_EVT_TTS_DATA:
            skillCollectTts(data, len);
            break;
        case AI_USER_EVT_TTS_STOP:
            skillSaveTts();
            break;
        case AI_USER_EVT_TEXT_STREAM_DATA:
            if (data && len > 0) Serial.write(data, len);
            break;
        case AI_USER_EVT_TEXT_STREAM_STOP:
            Serial.println();
            break;
        case AI_USER_EVT_SKILL:
            skillHandleEvent(data);
            break;
        case AI_USER_EVT_EMOTION:
        case AI_USER_EVT_LLM_EMOTION:
            skillHandleEmotion((AI_AGENT_EMO_T *)data);
            break;
        case AI_USER_EVT_PLAY_CTL_PLAY:
            Serial.println("[Play]");
            break;
        case AI_USER_EVT_PLAY_CTL_PAUSE:
            Serial.println("[Pause]");
            break;
        case AI_USER_EVT_PLAY_CTL_NEXT:
            Serial.println("[Next]");
            break;
        case AI_USER_EVT_PLAY_CTL_PREV:
            Serial.println("[Prev]");
            break;
        default:
            break;
    }
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
            PR_INFO("Device Bind Start!");
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
    // Other events: delegate to ai_mode_handle_key via TuyaAI
    TuyaAI.modeKeyHandle((uint8_t)event, arg);
}
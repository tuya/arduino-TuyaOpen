/**
 * @file ai_audio.ino
 * @brief Implements main audio functionality for IoT device
 * 
 * This source file provides the implementation of the main audio functionalities
 * required for an IoT device. It includes functionality for audio processing,
 * device initialization, event handling, and network communication. The
 * implementation supports audio volume control, data point processing, and
 * interaction with the Tuya IoT platform. This file is essential for developers
 * working on IoT applications that require audio capabilities and integration
 * with the Tuya IoT ecosystem.
 * 
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 *
 * @note ===================== AI audio only support TUYA_T5AI platform =====================
 */
#include "TuyaAI.h"
#include "TuyaIoT.h"
#include "Log.h"

#include "cJSON.h"
/***********************************************************
************************macro define************************
***********************************************************/
#define TUYA_DEVICE_UUID    "uuidxxxxxxxxxxxxxxxx"
#define TUYA_DEVICE_AUTHKEY "keyxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
#define TUYA_PRODUCT_ID     "alon7qgyjj8yus74"

#define AI_AUDIO_WORK_MODE AI_AUDIO_WORK_ASR_WAKEUP_SINGLE_TALK // choice work mode：provide 4 modes to chat with AI Agent

#define LED_PIN     1
#define BTN_PIN     12
#define TEXT_SIZE   256
#define DPID_VOLUME 3
/***********************************************************
***********************variable define**********************
***********************************************************/
TuyaAI TuyaAI;

uint8_t _recv_buf[TEXT_SIZE];
static uint8_t _need_reset = 0;
static uint8_t _chat_flag = 0;
/***********************************************************
***********************function define**********************
***********************************************************/
static void tuyaAIStateCallback(AI_AUDIO_STATE_E state);
static void tuyaAIEventCallback(AI_AUDIO_EVENT_E event, uint8_t *data, uint32_t len, void *arg);
static void tuyaIoTEventCallback(tuya_event_msg_t *event);
static void handleUserInput();

void setup() {
  OPERATE_RET rt = OPRT_OK;

  // Initialize logging
  Serial.begin(115200);
  Log.begin();

  PR_NOTICE("========================================");
  PR_NOTICE("AI Audio Example");
  PR_NOTICE("Project name:        %s", PROJECT_NAME);
  PR_NOTICE("App version:         %s", PROJECT_VERSION);
  PR_NOTICE("Compile time:        %s", __DATE__);
  PR_NOTICE("TuyaOpen version:    %s", OPEN_VERSION);
  PR_NOTICE("Platform chip:       %s", PLATFORM_CHIP);
  PR_NOTICE("Platform board:      %s", PLATFORM_BOARD);
  PR_NOTICE("========================================");

  // Pressing the reset button three times can clear the network configuration information
  TuyaIoT.resetNetcfg();

  // Hardware initialization
  rt = board_register_hardware();
  if (rt != OPRT_OK) {
    PR_ERR("board register error");
  }

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  TuyaIoT.setEventCallback(tuyaIoTEventCallback);
  TuyaIoT.setLicense(TUYA_DEVICE_UUID, TUYA_DEVICE_AUTHKEY);
  TuyaIoT.begin(TUYA_PRODUCT_ID, "1.0.0");

  // Initialize AI components
  AI_AUDIO_CONFIG_T ai_audio_cfg = {
    .work_mode = AI_AUDIO_WORK_MODE,  // choice work mode
    .evt_inform_cb = tuyaAIEventCallback,
    .state_inform_cb = tuyaAIStateCallback,
  };
  TuyaAI.begin(ai_audio_cfg);

  delay(2000); // wait iot init successfully
  TuyaIoT.resetNetconfigCheck();
}

void loop() {
  handleUserInput();
  
  if (0 == _chat_flag) {
    Serial.println("\nPlease enter information and chat with your AI Agent ...");
    _chat_flag = 1;
  }
  delay(10);
}

void handleUserInput() {
  static int i = 0;
  while (Serial.available()) {
    char c = Serial.read();
    _recv_buf[i++] = c;
    if (c == '\n' || c == '\r') {
      TuyaAI.textInput(_recv_buf, i);
      Serial.print("🧍‍♂️ User: ");
      Serial.write(_recv_buf, i);
      i = 0;
    }
  }
}

OPERATE_RET audio_dp_obj_proc(dp_obj_recv_t *dpobj) {
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
          break;
        }
      default:
        break;
    }
  }

  return OPRT_OK;
}

OPERATE_RET ai_audio_volume_upload(void) {
  uint8_t volume = TuyaAI.getVolume();
  PR_DEBUG("DP upload volume:%d", volume);
  return TuyaIoT.write(DPID_VOLUME, &volume, 1, 0);
}

void user_upgrade_notify_on(tuya_iot_client_t *client, cJSON *upgrade) {
  PR_INFO("----- Upgrade information -----");
  PR_INFO("OTA Channel: %d", cJSON_GetObjectItem(upgrade, "type")->valueint);
  PR_INFO("Version: %s", cJSON_GetObjectItem(upgrade, "version")->valuestring);
  PR_INFO("Size: %s", cJSON_GetObjectItem(upgrade, "size")->valuestring);
  PR_INFO("MD5: %s", cJSON_GetObjectItem(upgrade, "md5")->valuestring);
  PR_INFO("HMAC: %s", cJSON_GetObjectItem(upgrade, "hmac")->valuestring);
  PR_INFO("URL: %s", cJSON_GetObjectItem(upgrade, "url")->valuestring);
  PR_INFO("HTTPS URL: %s", cJSON_GetObjectItem(upgrade, "httpsUrl")->valuestring);
}

// ========== Tuya IoT Event Handler ==========
void tuyaIoTEventCallback(tuya_event_msg_t *event) {

  PR_DEBUG("Tuya Event ID:%d(%s)", event->id, EVENT_ID2STR(event->id));
  PR_INFO("Device Free heap %d", tal_system_get_free_heap_size());

  switch (event->id) {
    case TUYA_EVENT_BIND_START:
      PR_INFO("Device Bind Start!");
      if (_need_reset == 1) {
        PR_INFO("Device Reset!");
        tal_system_reset();
      }

      break;

    case TUYA_EVENT_BIND_TOKEN_ON:
      break;

    /* MQTT with tuya cloud is connected, device online */
    case TUYA_EVENT_MQTT_CONNECTED:
      PR_INFO("Device MQTT Connected!");
      tal_event_publish(EVENT_MQTT_CONNECTED, NULL);

      static uint8_t first = 1;
      if (first) {
        first = 0;
      }
      ai_audio_volume_upload();
      break;

    /* MQTT with tuya cloud is disconnected, device offline */
    case TUYA_EVENT_MQTT_DISCONNECT:
      PR_INFO("Device MQTT DisConnected!");
      tal_event_publish(EVENT_MQTT_DISCONNECTED, NULL);
      break;

    /* _recv_buf upgrade request */
    case TUYA_EVENT_UPGRADE_NOTIFY:
      PR_DEBUG("TUYA_EVENT_UPGRADE_NOTIFY");
      user_upgrade_notify_on(tuya_iot_client_get(), event->value.asJSON);
      break;

    /* Sync time with tuya Cloud */
    case TUYA_EVENT_TIMESTAMP_SYNC:
      PR_INFO("Sync timestamp:%d", event->value.asInteger);
      tal_time_set_posix(event->value.asInteger, 1);
      break;

    case TUYA_EVENT_RESET:
      PR_INFO("Device Reset:%d", event->value.asInteger);

      _need_reset = 1;
      break;

    /* recv OBJ DP */
    case TUYA_EVENT_DP_RECEIVE_OBJ:
      {
        dp_obj_recv_t *dpobj = event->value.dpobj;
        PR_DEBUG("SOC Rev DP Cmd t1:%d t2:%d CNT:%u", dpobj->cmd_tp, dpobj->dtt_tp, dpobj->dpscnt);
        if (dpobj->devid != NULL) {
          PR_DEBUG("devid.%s", dpobj->devid);
        }

        audio_dp_obj_proc(dpobj);
        TuyaIoT.write((dpobj->dps->id), (dpobj->dps->value), 0);
      }
      break;

    /* recv RAW DP */
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

static void tuyaAIEventCallback(AI_AUDIO_EVENT_E event, uint8_t *data, uint32_t len, void *arg) {
  switch (event) {
    case AI_AUDIO_EVT_HUMAN_ASR_TEXT:
      {
      }
      break;

    case AI_AUDIO_EVT_AI_REPLIES_TEXT_START:
      {
        _chat_flag = 1;
        Serial.print("AI: ");
      }
      break;

    case AI_AUDIO_EVT_AI_REPLIES_TEXT_DATA:
      {
        _chat_flag = 1;
        Serial.write(data, len);
      }
      break;

    case AI_AUDIO_EVT_AI_REPLIES_TEXT_END:
      {
        _chat_flag = 0;
        Serial.println("\n================chat end=================\n");
      }
      break;

    case AI_AUDIO_EVT_AI_REPLIES_EMO:
      {
        _chat_flag = 1;
        AI_AUDIO_EMOTION_T *emo;
        PR_DEBUG("---> AI_MSG_TYPE_EMOTION");
        emo = (AI_AUDIO_EMOTION_T *)data;
        if (emo) {
          if (emo->name) {
            PR_DEBUG("emotion name:%s", emo->name);
          }
          if (emo->text) {
            Serial.print(emo->text);
            Serial.print(" ");
          }
        }
      }
      break;

    case AI_AUDIO_EVT_ASR_WAKEUP:
      {
        TuyaAI.stopPlaying();
      }
      break;

    default:
      break;
  }
  return;
}

static void tuyaAIStateCallback(AI_AUDIO_STATE_E state) {
  PR_DEBUG("ai audio state: %d", state);
  switch (state) {
    case AI_AUDIO_STATE_STANDBY:
      break;
    case AI_AUDIO_STATE_LISTEN:
      break;
    case AI_AUDIO_STATE_UPLOAD:
      break;
    case AI_AUDIO_STATE_AI_SPEAK:
      break;
    default:
      break;
  }
}

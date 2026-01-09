/**
 * @file your_chat_bot.ino
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
 * @note ===================== Chat bot only support TUYA_T5AI platform =====================
 */
#include "TuyaAI.h"
#include "TuyaIoT.h"
#include "Log.h"
#include "Button.h"

#include "app_display.h"
#include "app_system_info.h"

#include "cJSON.h"
#include "netmgr.h"
/***********************************************************
************************macro define************************
***********************************************************/
#define TUYA_DEVICE_UUID    "uuidxxxxxxxxxxxxxxxx"
#define TUYA_DEVICE_AUTHKEY "keyxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
#define TUYA_PRODUCT_ID     "alon7qgyjj8yus74"

#define LANG_CODE_ZH 0  // Alert：1: Chinese, 0: English
#define AI_AUDIO_WORK_MODE AI_AUDIO_WORK_ASR_WAKEUP_SINGLE_TALK // choice work mode：provide 4 modes to chat with AI Agent

#define LED_PIN     1
#define BUTTON_PIN  12
#define TEXT_SIZE   256
#define DPID_VOLUME 3
/***********************************************************
***********************variable define**********************
***********************************************************/
TuyaAI TuyaAI;
Button Button0;

static uint8_t _recv_buf[TEXT_SIZE];
static uint8_t _need_reset = 0;
static uint8_t _chat_flag = 0;
/***********************************************************
***********************function define**********************
***********************************************************/
static void tuyaAIStateCallback(AI_AUDIO_STATE_E state);
static void tuyaAIEventCallback(AI_AUDIO_EVENT_E event, uint8_t *data, uint32_t len, void *arg);
static void tuyaIoTEventCallback(tuya_event_msg_t *event);
static void buttonEventCallback(char *name, ButtonEvent_t event, void *arg);
static void handleUserInput();
static void buttonEventInit();

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

  rt = app_display_init();
  if (rt != OPRT_OK) {
    PR_ERR("display error");
  }

  app_system_info();
  buttonEventInit();

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

  if (false == _chat_flag) {
    Serial.println("\nPlease enter information and chat with your AI Agent ...");
    _chat_flag = 1;
  }
  delay(10);
}

static void handleUserInput() {
  static int i = 0;
  while (Serial.available()) {
    char c = Serial.read();
    _recv_buf[i++] = c;
    if (c == '\n' || c == '\r') {
      TuyaAI.textInput(_recv_buf, i);
      Serial.print("\n 🧍‍♂️ User: ");
      Serial.write(_recv_buf, i);
      i = 0;
    }
  }
}

static void buttonEventCallback(char *name, ButtonEvent_t event, void *arg) {
  APP_CHAT_MODE_E work_mode = TuyaAI.getWorkMode();
  PR_DEBUG("app button function cb, work mode: %d", work_mode);

  // network status
  netmgr_status_e status = NETMGR_LINK_DOWN;
  netmgr_conn_get(NETCONN_AUTO, NETCONN_CMD_STATUS, &status);
  if (status == NETMGR_LINK_DOWN) {
    PR_DEBUG("network is down, ignore button event");
    if (TuyaAI.isPlaying()) {
      return;
    }
    TuyaAI.playAlert(AI_AUDIO_ALERT_NOT_ACTIVE);
    return;
  }

  switch (event) {
    case BUTTON_EVENT_PRESS_DOWN:
      {
        if (work_mode == APP_CHAT_MODE_KEY_PRESS_HOLD_SINGLE) {
          PR_DEBUG("button press down, listen start");
#if defined(ENABLE_LED) && (ENABLE_LED == 1)
          digitalWrite(LED_PIN, HIGH);
#endif
          TuyaAI.manualStartSingleTalk();
        }
      }
      break;
    case BUTTON_EVENT_PRESS_UP:
      {
        if (work_mode == APP_CHAT_MODE_KEY_PRESS_HOLD_SINGLE) {
          PR_DEBUG("button press up, listen end");
#if defined(ENABLE_LED) && (ENABLE_LED == 1)
          digitalWrite(LED_PIN, LOW);
#endif
          TuyaAI.manualStopSingleTalk();
        }
      }
      break;
    case BUTTON_EVENT_SINGLE_CLICK:
      {
        if (work_mode == APP_CHAT_MODE_KEY_PRESS_HOLD_SINGLE) {
          break;
        }
        TuyaAI.stopPlaying();
        TuyaAI.playAlert(AI_AUDIO_ALERT_WAKEUP);
        TuyaAI.setWakeup();

        PR_DEBUG("button single click");
      }
      break;

    default:
      break;
  }
}

static void buttonEventInit(void) {

  ButtonConfig_t cfg;
  cfg.debounceTime = 50;        // button debounce 50ms
  cfg.longPressTime = 2000;     // long press threshold 2000ms
  cfg.longPressHoldTime = 500;  // long press hold interval 500ms
  cfg.multiClickCount = 2;      // max multi-click count 2
  cfg.multiClickInterval = 500; // multi-click time window 500ms
  PinConfig_t pinCfg0;
  pinCfg0.pin = BUTTON_PIN;
  pinCfg0.level = TUYA_GPIO_LEVEL_LOW;
  pinCfg0.pullMode = TUYA_GPIO_PULLUP;

  Button0.begin("Button0", pinCfg0, cfg);
  Button0.setEventCallback(BUTTON_EVENT_PRESS_DOWN, buttonEventCallback);
  Button0.setEventCallback(BUTTON_EVENT_PRESS_UP, buttonEventCallback);
  Button0.setEventCallback(BUTTON_EVENT_SINGLE_CLICK, buttonEventCallback);
  Button0.setEventCallback(BUTTON_EVENT_DOUBLE_CLICK, buttonEventCallback);
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
#if defined(ENABLE_CHAT_DISPLAY) && (ENABLE_CHAT_DISPLAY == 1)
          char volume_str[20] = { 0 };
          snprintf(volume_str, sizeof(volume_str), "%s%d", VOLUME, volume);
          app_display_send_msg(TY_DISPLAY_TP_NOTIFICATION, (uint8_t *)volume_str, strlen(volume_str));
#endif
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

      TuyaAI.playAlert(AI_AUDIO_ALERT_NETWORK_CFG);
      break;

    case TUYA_EVENT_DIRECT_MQTT_CONNECTED:
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
#if defined(ENABLE_CHAT_DISPLAY) && (ENABLE_CHAT_DISPLAY == 1)
        UI_WIFI_STATUS_E wifi_status = UI_WIFI_STATUS_GOOD;
        app_display_send_msg(TY_DISPLAY_TP_NETWORK, (uint8_t *)&wifi_status, sizeof(UI_WIFI_STATUS_E));
#endif

        TuyaAI.playAlert(AI_AUDIO_ALERT_NETWORK_CONNECTED);
        ai_audio_volume_upload();
      }
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
#if defined(ENABLE_CHAT_DISPLAY) && (ENABLE_CHAT_DISPLAY == 1)
#if !defined(ENABLE_GUI_STREAM_AI_TEXT) || (ENABLE_GUI_STREAM_AI_TEXT != 1)
  static uint8_t *p_ai_text = NULL;
  static uint32_t ai_text_len = 0;
#endif
#endif

  switch (event) {
    case AI_AUDIO_EVT_HUMAN_ASR_TEXT:
      {
        if (len > 0 && data) {
          // send asr text to display
#if defined(ENABLE_CHAT_DISPLAY) && (ENABLE_CHAT_DISPLAY == 1)
          app_display_send_msg(TY_DISPLAY_TP_USER_MSG, data, len);
#else
          PR_NOTICE("USER: %.*s", (int)len, data);
#endif
        }
      }
      break;

    case AI_AUDIO_EVT_AI_REPLIES_TEXT_START:
      {
#if defined(ENABLE_CHAT_DISPLAY) && (ENABLE_CHAT_DISPLAY == 1)
#if defined(ENABLE_GUI_STREAM_AI_TEXT) && (ENABLE_GUI_STREAM_AI_TEXT == 1)
        app_display_send_msg(TY_DISPLAY_TP_ASSISTANT_MSG_STREAM_START, data, len);
#else
        if (NULL == p_ai_text) {
          p_ai_text = tkl_system_psram_malloc(AI_AUDIO_TEXT_BUFF_LEN);
          if (NULL == p_ai_text) {
            return;
          }
        }

        ai_text_len = 0;
#endif
#else
        PR_NOTICE("AI: ", len);
#endif
        _chat_flag = 1;
        Serial.print("\n AI: ");
      }
      break;

    case AI_AUDIO_EVT_AI_REPLIES_TEXT_DATA:
      {
#if defined(ENABLE_CHAT_DISPLAY) && (ENABLE_CHAT_DISPLAY == 1)
#if defined(ENABLE_GUI_STREAM_AI_TEXT) && (ENABLE_GUI_STREAM_AI_TEXT == 1)
        app_display_send_msg(TY_DISPLAY_TP_ASSISTANT_MSG_STREAM_DATA, data, len);
#else
        memcpy(p_ai_text + ai_text_len, data, len);

        ai_text_len += len;
        if (ai_text_len >= AI_AUDIO_TEXT_SHOW_LEN) {
          app_display_send_msg(TY_DISPLAY_TP_ASSISTANT_MSG, p_ai_text, ai_text_len);
          ai_text_len = 0;
        }
#endif
#else
        PR_NOTICE("AI: %.*s", len, data);
#endif
        _chat_flag = 1;
        Serial.write(data, len);
      }
      break;

    case AI_AUDIO_EVT_AI_REPLIES_TEXT_END:
      {
#if defined(ENABLE_CHAT_DISPLAY) && (ENABLE_CHAT_DISPLAY == 1)
#if defined(ENABLE_GUI_STREAM_AI_TEXT) && (ENABLE_GUI_STREAM_AI_TEXT == 1)
        app_display_send_msg(TY_DISPLAY_TP_ASSISTANT_MSG_STREAM_END, data, len);
#else
        app_display_send_msg(TY_DISPLAY_TP_ASSISTANT_MSG, p_ai_text, ai_text_len);
        ai_text_len = 0;
#endif
#endif
        _chat_flag = 0;
        Serial.println("\n================chat end=================\n");
      }
      break;

    case AI_AUDIO_EVT_AI_REPLIES_TEXT_INTERUPT:
      {
#if defined(ENABLE_GUI_STREAM_AI_TEXT) && (ENABLE_GUI_STREAM_AI_TEXT == 1)
        app_display_send_msg(TY_DISPLAY_TP_ASSISTANT_MSG_STREAM_INTERRUPT, NULL, 0);
#else
        PR_WARN("AI response interrupted");
#endif
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
#if defined(ENABLE_CHAT_DISPLAY) && (ENABLE_CHAT_DISPLAY == 1)
            app_display_send_msg(TY_DISPLAY_TP_EMOTION, (uint8_t *)emo->name, strlen(emo->name));
#endif
          }

          if (emo->text) {
            PR_DEBUG("emotion text:%s", emo->text);
            Serial.print(emo->text);
            Serial.print(" ");
          }
        }
      }
      break;

    case AI_AUDIO_EVT_ASR_WAKEUP:
      {
        TuyaAI.stopPlaying();
        TuyaAI.playAlert(AI_AUDIO_ALERT_WAKEUP);

#if defined(ENABLE_LED) && (ENABLE_LED == 1)
        digitalWrite(LED_PIN, LOW);
#endif

#if defined(ENABLE_GUI_STREAM_AI_TEXT) && (ENABLE_GUI_STREAM_AI_TEXT == 1)
        app_display_send_msg(TY_DISPLAY_TP_ASSISTANT_MSG_STREAM_END, data, len);
#endif
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

#if defined(ENABLE_LED) && (ENABLE_LED == 1)
      digitalWrite(LED_PIN, LOW);
#endif

#if defined(ENABLE_CHAT_DISPLAY) && (ENABLE_CHAT_DISPLAY == 1)
      app_display_send_msg(TY_DISPLAY_TP_EMOTION, (uint8_t *)EMOJI_NEUTRAL, strlen(EMOJI_NEUTRAL));
      app_display_send_msg(TY_DISPLAY_TP_STATUS, (uint8_t *)STANDBY, strlen(STANDBY));
#else
      PR_NOTICE("State: STANDBY (Ready for next conversation)");
#endif
      break;
    case AI_AUDIO_STATE_LISTEN:
#if defined(ENABLE_LED) && (ENABLE_LED == 1)
      digitalWrite(LED_PIN, HIGH);
#endif

#if defined(ENABLE_CHAT_DISPLAY) && (ENABLE_CHAT_DISPLAY == 1)
      app_display_send_msg(TY_DISPLAY_TP_STATUS, (uint8_t *)LISTENING, strlen(LISTENING));
#else
      PR_NOTICE("State: LISTENING (Recording audio...)");
#endif
      break;
    case AI_AUDIO_STATE_UPLOAD:
#if !defined(ENABLE_CHAT_DISPLAY) || (ENABLE_CHAT_DISPLAY != 1)
      PR_NOTICE("State: UPLOAD (Sending to cloud...)");
#endif
      break;

    case AI_AUDIO_STATE_AI_SPEAK:
#if defined(ENABLE_CHAT_DISPLAY) && (ENABLE_CHAT_DISPLAY == 1)
      app_display_send_msg(TY_DISPLAY_TP_STATUS, (uint8_t *)SPEAKING, strlen(SPEAKING));
#else
      PR_NOTICE("State: AI_SPEAKING (Playing response...)");
#endif
      break;

    default:
      break;
  }
}

#include "TuyaAI.h"
#include "TuyaIoT.h"
#include "Log.h"

#define TUYA_DEVICE_UUID "uuid66a64f11832c16c2"
#define TUYA_DEVICE_AUTHKEY "ah0hvQi5C8D1AthHihYYOM7vjAQTuRh7"
#define TUYA_PRODUCT_ID "alon7qgyjj8yus74"
#define LED_PIN 9
#define DPID_VOLUME 3
#define TEXT_SIZE 256

TuyaAI TuyaAI;

uint8_t recv[TEXT_SIZE];
static uint8_t _need_reset = 0;
static uint8_t _chat_flag = 0;

// static void __ai_audio_agent_event_cb(AI_EVENT_TYPE event, AI_EVENT_ID event_id);
// static void __ai_audio_agent_msg_cb(AI_AGENT_MSG_T *msg);
static void __app_ai_audio_state_inform_cb(AI_AUDIO_STATE_E state);
static void __app_ai_audio_evt_inform_cb(AI_AUDIO_EVENT_E event, uint8_t *data, uint32_t len, void *arg);
void tuyaIoTEventCallback(tuya_event_msg_t *event);
void handleUserInput();
OPERATE_RET ai_audio_player_play_alert(AI_AUDIO_ALERT_TYPE_E type);

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

  rt = board_register_hardware();
  if (rt != OPRT_OK) {
    PR_ERR("board register error");
  }

  // Hardware initialization
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  TuyaIoT.setEventCallback(tuyaIoTEventCallback);
  TuyaIoT.setLicense(TUYA_DEVICE_UUID, TUYA_DEVICE_AUTHKEY);

  // Initialize AI components
  AI_AUDIO_CONFIG_T ai_audio_cfg = {
    .work_mode = AI_AUDIO_WORK_ASR_WAKEUP_FREE_TALK,
    .evt_inform_cb = __app_ai_audio_evt_inform_cb,
    .state_inform_cb = __app_ai_audio_state_inform_cb,
  };
  TuyaAI.begin(ai_audio_cfg);

  TuyaIoT.begin(TUYA_PRODUCT_ID, "1.0.0");
}

void loop() {
  // Main loop can be empty for this example
  handleUserInput();
  // PR_INFO("Device Free heap %d", tal_system_get_free_heap_size());
  if (0 == _chat_flag) {
    Serial.println("\nPlease enter information and chat with your AI Agent ...");
    _chat_flag = 1;
  }
  digitalWrite(LED_PIN, LOW);
  delay(100);
  digitalWrite(LED_PIN, HIGH);
}

void handleUserInput() {
  static int i = 0;
  while (Serial.available()) {
    char c = Serial.read();
    recv[i++] = c;
    if (c == '\n' || c == '\r') {
      TuyaAI.textInput(recv, i);
      Serial.print("🧍‍♂️ User: ");
      Serial.write(recv, i);
      i = 0;
    }
  }
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
      break;

    /* MQTT with tuya cloud is disconnected, device offline */
    case TUYA_EVENT_MQTT_DISCONNECT:
      PR_INFO("Device MQTT DisConnected!");
      tal_event_publish(EVENT_MQTT_DISCONNECTED, NULL);
      break;

    /* RECV upgrade request */
    case TUYA_EVENT_UPGRADE_NOTIFY:
      PR_DEBUG("TUYA_EVENT_UPGRADE_NOTIFY");
    //   user_upgrade_notify_on(event->value.asJSON);
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

    /* RECV OBJ DP */
    case TUYA_EVENT_DP_RECEIVE_OBJ:
      {
        dp_obj_recv_t *dpobj = event->value.dpobj;
        PR_DEBUG("SOC Rev DP Cmd t1:%d t2:%d CNT:%u", dpobj->cmd_tp, dpobj->dtt_tp, dpobj->dpscnt);
        if (dpobj->devid != NULL) {
          PR_DEBUG("devid.%s", dpobj->devid);
        }

        // audio_dp_obj_proc(dpobj);
        TuyaIoT.write((dpobj->dps->id), (dpobj->dps->value), 0);
      }
      break;

    /* RECV RAW DP */
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

static void __app_ai_audio_evt_inform_cb(AI_AUDIO_EVENT_E event, uint8_t *data, uint32_t len, void *arg) {
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
            // PR_DEBUG("emotion text:%s", emo->text);
            Serial.print(emo->text);
            Serial.print(" ");
          }
        }
      }
      break;

    case AI_AUDIO_EVT_ASR_WAKEUP:
      {
        ai_audio_player_stop();
        // ai_audio_player_play_alert(AI_AUDIO_ALERT_WAKEUP);
      }
      break;

    default:
      break;
  }
  return;
}

static void __app_ai_audio_state_inform_cb(AI_AUDIO_STATE_E state) {
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

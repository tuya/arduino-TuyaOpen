/**
 * @file YourChatBot.ino
 * @brief AI ChatBot example using TuyaAI
 * @copyright Copyright (c) 2021-2026 Tuya Inc. All Rights Reserved.
 * @note Only supports TUYA_T5AI platform
 */

#include <Arduino.h>
#include "TuyaAI.h"
#include "TuyaIoT.h"
#include "Log.h"
#include "lang_config.h"

#include "cJSON.h"
#include "netmgr.h"

#include "appButton.h"
#include "appDisplay.h"
#include "appMCP.h"
#include "appStatus.h"
#include "appAudioRecord.h"
/***********************************************************
************************macro define************************
***********************************************************/
// Device credentials (replace with your own)
#define TUYA_DEVICE_UUID    "uuidxxxxxxxxxxxxxxxx"
#define TUYA_DEVICE_AUTHKEY "keyxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
#define TUYA_PRODUCT_ID     "9inb01mvjqh5zhhr"

// Audio recording configuration
#define ENABLE_AUDIO_RECORDING 0 // Set to 1 to enable MIC/TTS recording to SD card

// Hardware pins
#define LED_PIN    1
#define BUTTON_PIN 12

#define DPID_VOLUME 3
/***********************************************************
***********************static declarations******************
***********************************************************/
static void tuyaIoTEventCallback(tuya_event_msg_t *event);
static void aiEventCallback(AI_USER_EVT_TYPE_E event, uint8_t *data, uint32_t len, void *arg);
static void aiStateCallback(AI_MODE_STATE_E state);
static void handleUserInput();
/***********************************************************
***********************variable define**********************
***********************************************************/
static uint8_t gNeedReset = 0;
/***********************************************************
***********************public functions*********************
***********************************************************/

void setup()
{
    // Initialize Serial and logging
    Serial.begin(115200);
    Log.begin();
    Log.setLevel(LogClass::WARN);

    // Print startup banner
    PR_NOTICE("============Your Chat Bot==============");
    PR_NOTICE("Compile time:        %s", __DATE__);

    // Hardware initialization
    if (OPRT_OK != board_register_hardware()) {
        PR_ERR("Board hardware registration failed");
    }

    // Initialize LED pin
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    // Setup TuyaIoT
    TuyaIoT.resetNetcfg(); // Reset network config on 3 quick rst button presses
    TuyaIoT.setEventCallback(tuyaIoTEventCallback);
    TuyaIoT.setLicense(TUYA_DEVICE_UUID, TUYA_DEVICE_AUTHKEY);
    TuyaIoT.begin(TUYA_PRODUCT_ID, PROJECT_VERSION);

    // Initialize TuyaAI core
    AIConfig_t aiConfig = {AI_CHAT_MODE_WAKEUP, 70, aiEventCallback, aiStateCallback, NULL};
    if (OPRT_OK != TuyaAI.begin(aiConfig)) {
        PR_ERR("TuyaAI initialization failed");
    }

    // Initialize Audio
    if (OPRT_OK != TuyaAI.Audio.begin()) {
        PR_ERR("TuyaAI Audio initialization failed");
    }

    // Initialize UI subsystem
    if (OPRT_OK != appDisplayInit(UI_TYPE)) {
        PR_ERR("UI initialization failed");
    }

    // Initialize button handling
    if (OPRT_OK != appButtonInit(BUTTON_PIN)) {
        PR_ERR("Button initialization failed");
    }

    // Initialize status monitoring (WiFi status, heap print, etc.)
    if (OPRT_OK != appStatusInit()) {
        PR_ERR("Status monitoring initialization failed");
    }

    if (OPRT_OK != appMCPInit()) {
        PR_ERR("MCP initialization failed (may not be enabled)");
    }

#if ENABLE_AUDIO_RECORDING
    // Initialize audio recording to SD card
    appAudioRecordInit();
#endif

    PR_NOTICE("Setup complete, waiting for network...");
    // Wait for IoT initialization
    delay(2000);
    TuyaIoT.resetNetconfigCheck();
}

void loop()
{
    handleUserInput();

#if ENABLE_AUDIO_RECORDING
    // Flush buffered audio data to SD card
    appAudioRecordFlush();
#endif

    delay(10);
}

/***********************************************************
***********************static implementations***************
***********************************************************/

/**
 * @brief AI event callback
 * Handle AI events like ASR results, TTS, emotions, etc.
 * Directly updates display using appDisplay functions (pure LVGL)
 */
static void aiEventCallback(AI_USER_EVT_TYPE_E event, uint8_t *data, uint32_t len, void *arg)
{
    switch (event) {
    case AI_USER_EVT_IDLE:
        // Internal idle state, no data
        Serial.println("\n[EVT] AI_USER_EVT_IDLE");
        break;

    case AI_USER_EVT_ASR_EMPTY:
        // ASR result is empty (user didn't speak)
        Serial.println("\n[EVT] AI_USER_EVT_ASR_EMPTY - ASR result empty");
        break;

    case AI_USER_EVT_ASR_OK:
        // ASR recognition successful - Cloud ASR result
        Serial.print("\n[EVT] AI_USER_EVT_ASR_OK");
        if (data != nullptr && len > 0) {
            Serial.print(" - User said: ");
            Serial.write(data, len);
            appDisplaySetUserText((const char *)data);
        }
        break;

    case AI_USER_EVT_ASR_ERROR:
        // ASR recognition error, no data
        Serial.println("\n[EVT] AI_USER_EVT_ASR_ERROR - ASR failed");
        break;

    case AI_USER_EVT_MIC_DATA:
        // Raw microphone PCM audio data for processing
#if ENABLE_AUDIO_RECORDING
        // Save MIC PCM data to file
        if (data != nullptr && len > 0) {
            appMicRecordWrite((const uint8_t *)data, len);
        }
#endif
        break;

    case AI_USER_EVT_TTS_PRE:
        // TTS playback about to start, no data
        Serial.println("\n[EVT] AI_USER_EVT_TTS_PRE - TTS preparing");
        break;

    case AI_USER_EVT_TTS_START:
        // TTS playback started, no data
        Serial.println("\n[EVT] AI_USER_EVT_TTS_START - TTS playing");
#if ENABLE_AUDIO_RECORDING
        appTtsRecordStart();
#endif
        break;

    case AI_USER_EVT_TTS_DATA:
        // Cloud TTS audio data chunk
#if ENABLE_AUDIO_RECORDING
        // Save TTS audio data to file (MP3 format)
        if (data != nullptr && len > 0) {
            appTtsRecordWrite((const uint8_t *)data, len);
        }
#endif
        break;

    case AI_USER_EVT_TTS_STOP:
        // TTS playback stopped normally, no data
        Serial.println("\n[EVT] AI_USER_EVT_TTS_STOP - TTS stopped");
#if ENABLE_AUDIO_RECORDING
        appTtsRecordStop();
#endif
        break;

    case AI_USER_EVT_TTS_ABORT:
        // TTS playback aborted (interrupted), no data
        Serial.println("\n[EVT] AI_USER_EVT_TTS_ABORT - TTS aborted");
#if ENABLE_AUDIO_RECORDING
        appTtsRecordStop();
#endif
        break;

    case AI_USER_EVT_TTS_ERROR:
        // TTS playback error, no data
        Serial.println("\n[EVT] AI_USER_EVT_TTS_ERROR - TTS error");
        break;

    case AI_USER_EVT_VAD_TIMEOUT:
        // Voice Activity Detection timeout, no data
        Serial.println("\n[EVT] AI_USER_EVT_VAD_TIMEOUT - VAD timeout");
        break;

    case AI_USER_EVT_TEXT_STREAM_START:
        // AI text response stream started (includes MCP results)
        Serial.print("\n[EVT] AI_USER_EVT_TEXT_STREAM_START\n[AI Response]: ");
        if (data != nullptr && len > 0) {
            Serial.write(data, len);
            appDisplayStreamStart();
            appDisplaySetAIText((const char *)data);
        }
        break;

    case AI_USER_EVT_TEXT_STREAM_DATA:
        // AI text response streaming data chunk
        Serial.print("\n[AI Response]: ");
        if (data != nullptr && len > 0) {
            appDisplaySetAIText((const char *)data);
            Serial.write(data, len);
        }
        break;

    case AI_USER_EVT_TEXT_STREAM_STOP:
        // AI text response stream completed
        Serial.println("\n[EVT] AI_USER_EVT_TEXT_STREAM_STOP - Response complete");
        appDisplayStreamEnd();
        break;

    case AI_USER_EVT_TEXT_STREAM_ABORT:
        // AI text response stream aborted, no data
        Serial.println("\n[EVT] AI_USER_EVT_TEXT_STREAM_ABORT - Response aborted");
        break;

    case AI_USER_EVT_EMOTION:
        // Emotion detected from text tags
        Serial.print("\n[EVT] AI_USER_EVT_EMOTION");
        if (data != nullptr) {
            AI_AGENT_EMO_T *emo = (AI_AGENT_EMO_T *)data;
            Serial.print(" name:");
            Serial.print(emo->name ? emo->name : "null");

            if (emo->emoji) {
                char utf8[8];
                int  utf8Len = TuyaAI.Skill.unicodeToUtf8(emo->emoji, utf8, sizeof(utf8));
                if (utf8Len > 0) {
                    Serial.print(" emoji:");
                    Serial.print(utf8);
                }
            }
        }
        break;

    case AI_USER_EVT_LLM_EMOTION:
        // Emotion from LLM response
        Serial.print("\n[EVT] AI_USER_EVT_LLM_EMOTION");
        if (data != nullptr) {
            AI_AGENT_EMO_T *emo = (AI_AGENT_EMO_T *)data;
            Serial.print(" name:");
            Serial.print(emo->name ? emo->name : "null");

            if (emo->name && strcmp(emo->name, EMOJI_HAPPY) == 0) {
                Serial.print(" [Happy detected!]");
            }
        }
        break;

    case AI_USER_EVT_SKILL:
        // Data: cJSON* containing skill data
        Serial.print("\n[EVT] AI_USER_EVT_SKILL");
        if (data != nullptr) {
            SkillData_t skill;
            if (OPRT_OK == TuyaAI.Skill.parse(data, skill)) {
                Serial.print(" code:");
                Serial.print(skill.code);
                Serial.print(" type:");
                Serial.print(skill.type);

                switch (skill.type) {
                case SKILL_TYPE_MUSIC:
                case SKILL_TYPE_STORY:
                    if (skill.music != nullptr) {
                        Serial.print("\n[Music] action:");
                        Serial.print(skill.music->action);
                        Serial.print("\n[Music] count:");
                        Serial.print(skill.music->src_cnt);
                        Serial.print("\n[Music] url:");
                        Serial.print(skill.music->src_array[0].url);
                        TuyaAI.Skill.dumpMusic(skill);
                        TuyaAI.Skill.freeMusic(skill);
                    }
                    break;

                case SKILL_TYPE_PLAY_CONTROL:
                    Serial.print("\n[PlayControl] action:");
                    Serial.print(skill.playControl.action);
                    break;

                case SKILL_TYPE_EMOTION:
                    Serial.print("\n[Emotion skill]");
                    break;

                default:
                    break;
                }
            }
        }
        break;

    case AI_USER_EVT_CHAT_BREAK:
        // Chat interrupted by user or cloud, no data
        Serial.println("\n[EVT] AI_USER_EVT_CHAT_BREAK - Chat interrupted");
        break;

    case AI_USER_EVT_SERVER_VAD:
        // Server-side VAD detected speech end, no data
        Serial.println("\n[EVT] AI_USER_EVT_SERVER_VAD - Server VAD triggered");
        break;

    case AI_USER_EVT_END:
        // AI session ended, no data
        Serial.println("\n[EVT] AI_USER_EVT_END - Session ended");
        break;

    case AI_USER_EVT_EXIT:
        // AI chat exit requested, no data
        Serial.println("\n[EVT] AI_USER_EVT_EXIT - Chat exit");
        break;

    case AI_USER_EVT_PLAY_CTL_PLAY:
        // Music/audio playback started, no data
        Serial.println("\n[EVT] AI_USER_EVT_PLAY_CTL_PLAY - Playback started");
        break;

    case AI_USER_EVT_PLAY_CTL_RESUME:
        // Music/audio playback resumed, no data
        Serial.println("\n[EVT] AI_USER_EVT_PLAY_CTL_RESUME - Playback resumed");
        break;

    case AI_USER_EVT_PLAY_CTL_PAUSE:
        // Music/audio playback paused, no data
        Serial.println("\n[EVT] AI_USER_EVT_PLAY_CTL_PAUSE - Playback paused");
        break;

    case AI_USER_EVT_PLAY_CTL_REPLAY:
        // Replay current track, no data
        Serial.println("\n[EVT] AI_USER_EVT_PLAY_CTL_REPLAY - Replay");
        break;

    case AI_USER_EVT_PLAY_CTL_PREV:
        // Play previous track, no data
        Serial.println("\n[EVT] AI_USER_EVT_PLAY_CTL_PREV - Previous track");
        break;

    case AI_USER_EVT_PLAY_CTL_NEXT:
        // Play next track, no data
        Serial.println("\n[EVT] AI_USER_EVT_PLAY_CTL_NEXT - Next track");
        break;

    case AI_USER_EVT_PLAY_CTL_SEQUENTIAL:
        // Set sequential play mode (no loop), no data
        Serial.println("\n[EVT] AI_USER_EVT_PLAY_CTL_SEQUENTIAL - Sequential mode");
        break;

    case AI_USER_EVT_PLAY_CTL_SEQUENTIAL_LOOP:
        // Set sequential loop play mode, no data
        Serial.println("\n[EVT] AI_USER_EVT_PLAY_CTL_SEQUENTIAL_LOOP - Sequential loop mode");
        break;

    case AI_USER_EVT_PLAY_CTL_SINGLE_LOOP:
        // Set single track loop mode, no data
        Serial.println("\n[EVT] AI_USER_EVT_PLAY_CTL_SINGLE_LOOP - Single loop mode");
        break;

    case AI_USER_EVT_PLAY_CTL_END:
        // Play control command ended, no data
        Serial.println("\n[EVT] AI_USER_EVT_PLAY_CTL_END - Play control ended");
        break;

    case AI_USER_EVT_PLAY_END:
        // Audio playback finished, no data
        Serial.println("\n[EVT] AI_USER_EVT_PLAY_END - Playback finished");
        break;

    case AI_USER_EVT_PLAY_ALERT:
        // Alert notification from cloud
        {
            AI_AUDIO_ALERT_TYPE_E alertType = (AI_AUDIO_ALERT_TYPE_E)(intptr_t)data;
            Serial.print("\n[EVT] AI_USER_EVT_PLAY_ALERT - type:");
            Serial.print(alertType);
        }
        break;

    case AI_USER_EVT_MODE_SWITCH:
        // Chat mode switched
        // Modes: HOLD=0, ONESHOT=1, WAKEUP=2, FREE=3
        {
            int modeValue = (int)(intptr_t)data;
            Serial.print("\n[EVT] AI_USER_EVT_MODE_SWITCH - mode:");
            Serial.print(modeValue);
            appDisplaySetMode(modeValue);
        }
        break;

    case AI_USER_EVT_MODE_STATE_UPDATE:
        // Note: This is already handled by aiStateCallback, no need to process here
        break;

    default:
        Serial.print("\n[EVT] Unknown event: ");
        Serial.print(event);
        break;
    }
}

/**
 * @brief AI state change callback
 * Handle state transitions and update display
 */
static void aiStateCallback(AI_MODE_STATE_E state)
{
    const char *stateStr;
    switch (state) {
    case AI_MODE_STATE_IDLE:
        stateStr = STANDBY;
        break;
    case AI_MODE_STATE_INIT:
        stateStr = INITIALIZING;
        break;
    case AI_MODE_STATE_LISTEN:
        stateStr = LISTENING;
        break;
    case AI_MODE_STATE_UPLOAD:
        stateStr = "Uploading";
        break;
    case AI_MODE_STATE_THINK:
        stateStr = "Thinking";
        break;
    case AI_MODE_STATE_SPEAK:
        stateStr = SPEAKING;
        break;
    default:
        stateStr = STANDBY;
        break;
    }

    PR_DEBUG("AI State: %s", stateStr);
    appDisplaySetStatus(stateStr);

#if ENABLE_AUDIO_RECORDING
    // Start MIC recording when entering LISTENING state
    if (state == AI_MODE_STATE_LISTEN) {
        appMicRecordStart();
    }
    // Stop MIC recording when leaving LISTENING state
    else if (state == AI_MODE_STATE_UPLOAD || state == AI_MODE_STATE_THINK || state == AI_MODE_STATE_IDLE ||
             state == AI_MODE_STATE_INIT) {
        appMicRecordStop();
    }
#endif
}

void userUpgradeNotify(tuya_iot_client_t *client, cJSON *upgrade)
{
    PR_INFO("----- Upgrade information -----");
    PR_INFO("OTA Channel: %d", cJSON_GetObjectItem(upgrade, "type")->valueint);
    PR_INFO("Version: %s", cJSON_GetObjectItem(upgrade, "version")->valuestring);
    PR_INFO("Size: %s", cJSON_GetObjectItem(upgrade, "size")->valuestring);
    PR_INFO("MD5: %s", cJSON_GetObjectItem(upgrade, "md5")->valuestring);
    PR_INFO("HMAC: %s", cJSON_GetObjectItem(upgrade, "hmac")->valuestring);
    PR_INFO("URL: %s", cJSON_GetObjectItem(upgrade, "url")->valuestring);
    PR_INFO("HTTPS URL: %s", cJSON_GetObjectItem(upgrade, "httpsUrl")->valuestring);
}
OPERATE_RET aiAudioVolumUpload(void)
{
    uint8_t volume = TuyaAI.getVolume();
    PR_DEBUG("DP upload volume:%d", volume);
    return TuyaIoT.write(DPID_VOLUME, &volume, 1);
}

OPERATE_RET audioDpObjProc(dp_obj_recv_t *dpobj)
{
    uint32_t index = 0;
    for (index = 0; index < dpobj->dpscnt; index++) {
        dp_obj_t *dp = dpobj->dps + index;
        PR_DEBUG("idx:%d dpid:%d type:%d ts:%u", index, dp->id, dp->type, dp->time_stamp);

        switch (dp->id) {
        case DPID_VOLUME: {
            uint8_t volume = dp->value.dp_value;
            PR_DEBUG("volume:%d", volume);
            TuyaAI.setVolume(volume);
            char volume_str[20] = {0};
            snprintf(volume_str, sizeof(volume_str), "%s%d", VOLUME, volume);
            TuyaAI.UI.displayMessage(AI_UI_DISP_NOTIFICATION, (uint8_t *)volume_str, strlen(volume_str));
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
        TuyaAI.Audio.playAlert(AI_AUDIO_ALERT_NETWORK_CFG);
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
            appDisplaySetWifi(AI_UI_WIFI_STATUS_GOOD);
            aiAudioVolumUpload();
        }
        break;

    case TUYA_EVENT_MQTT_DISCONNECT:
        PR_INFO("Device MQTT Disconnected!");
        tal_event_publish(EVENT_MQTT_DISCONNECTED, NULL);
        appDisplaySetWifi(AI_UI_WIFI_STATUS_DISCONNECTED);
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

    case TUYA_EVENT_DP_RECEIVE_OBJ: {
        dp_obj_recv_t *dpobj = event->value.dpobj;
        PR_DEBUG("SOC Rev DP Cmd t1:%d t2:%d CNT:%u", dpobj->cmd_tp, dpobj->dtt_tp, dpobj->dpscnt);
        if (dpobj->devid != NULL) {
            PR_DEBUG("devid.%s", dpobj->devid);
        }

        audioDpObjProc(dpobj);
        TuyaIoT.write((dpobj->dps->id), (dpobj->dps->value), 0);
    } break;

    case TUYA_EVENT_DP_RECEIVE_RAW: {
        dp_raw_recv_t *dpraw = event->value.dpraw;
        PR_DEBUG("SOC Rev DP Cmd t1:%d t2:%d", dpraw->cmd_tp, dpraw->dtt_tp);
        if (dpraw->devid != NULL) {
            PR_DEBUG("devid.%s", dpraw->devid);
        }

        uint32_t  index = 0;
        dp_raw_t *dp    = &dpraw->dp;
        PR_DEBUG("dpid:%d type:RAW len:%d data:", dp->id, dp->len);
        for (index = 0; index < dp->len; index++) {
            PR_DEBUG_RAW("%02x", dp->data[index]);
        }

        TuyaIoT.write((dpraw->dp.id), (dpraw->dp.data), (dpraw->dp.len), 3);
    } break;

    default:
        break;
    }
}

static void handleUserInput()
{
    static int     i = 0;
    static uint8_t _recv_buf[256];
    while (Serial.available()) {
        char c         = Serial.read();
        _recv_buf[i++] = c;
        if (c == '\n' || c == '\r') {
            TuyaAI.sendText(_recv_buf, i);
            Serial.print("\n[User]: ");
            Serial.write(_recv_buf, i);
            TuyaAI.UI.displayText((const char *)_recv_buf, true);
            i = 0;
            memset(_recv_buf, 0, sizeof(_recv_buf));
        }
    }
}
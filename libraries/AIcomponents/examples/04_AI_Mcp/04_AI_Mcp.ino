/**
 * @file AI_Mcp.ino
 * @brief AI MCP - voice control LED using MCP tools
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 * @note Only supports TUYA_T5AI platform
 */

#include <Arduino.h>
#include "TuyaAI.h"
#include "TuyaIoT.h"
#include "Log.h"
#include "Button.h"

/***********************************************************
************************macro define************************
***********************************************************/
// Device credentials
#define TUYA_DEVICE_UUID    "uuid5ff159bdb94f3b8c"
#define TUYA_DEVICE_AUTHKEY "2MFUOajvyI8hPuGBgyTwCpIC7b4Du5f8"
#define TUYA_PRODUCT_ID     "9inb01mvjqh5zhhr"

#define LED_PIN             1
#define BUTTON_PIN          12

#define DPID_VOLUME         3
/***********************************************************
***********************variable define**********************
***********************************************************/
static Button gButton;
static bool gLedState = false;
static uint8_t gNeedReset = 0;
/***********************************************************
***********************static declarations******************
***********************************************************/
static void aiEventCallback(AI_USER_EVT_TYPE_E event, uint8_t *data, uint32_t len, void *arg);
static void tuyaIoTEventCallback(tuya_event_msg_t *event);
static void onButtonEvent(char *name, ButtonEvent_t event, void *arg);
static int initMCPTools(void* date);
static int onSetVolume(MCPPropertyList_t props, MCPReturnValue_t ret, void *userData);
static void handleUserInput();
/***********************************************************
***********************public functions*********************
***********************************************************/
void setup()
{
    Serial.begin(115200);
    Log.begin();
    // Log.setLevel(LogClass::WARN);
    
    // Print startup banner
    PR_NOTICE("============ Tuya AI MCP ==============");
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
    
    // Initialize MCP tools
    tal_event_subscribe(EVENT_MQTT_CONNECTED, "mcpRegisterTool", initMCPTools, SUBSCRIBE_TYPE_ONETIME);

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
    delay(10);
    handleUserInput();
}

/***********************************************************
***********************MCP Tools****************************
***********************************************************/
static int onSetVolume(MCPPropertyList_t props, MCPReturnValue_t ret, void *userData)
{
    // Get volume from properties
    int volume = TuyaAI.MCP.getPropertyInt(props, "volume", 50);
    PR_DEBUG("Setting volume to %d", volume);
    // Set volume using TuyaAI Audio component
    TuyaAI.Audio.setVolume(volume);
    // set return value
    TuyaAI.MCP.setReturnBool(ret, true);
    return 0;
}

static int initMCPTools(void* data)
{
    if (OPRT_OK != TuyaAI.MCP.begin()) {
        PR_ERR("MCP init failed");
        return -1;
    }
    
    // Register tool with integer property (range 0-100)
    TuyaMCPPropDef volumeProps[] = {
        TuyaMCPPropIntRange("volume", "The volume level to set (0-100).", 0, 100)
    };
    TUYA_MCP_TOOL_REGISTER(
        "device_audio_volume_set",
        "Sets the device's volume level.\n"
        "Parameters:\n"
        "- volume (int): The volume level to set (0-100).\n"
        "Response:\n"
        "- Returns true if the volume was set successfully.",
        onSetVolume,
        nullptr,
        volumeProps, 1
    );
    
    Serial.println("[MCP] Tools registered");
    return OPRT_OK;
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
        case AI_USER_EVT_TEXT_STREAM_START:
            // AI text response stream started (includes MCP results)
            Serial.print("\n[EVT] AI_USER_EVT_TEXT_STREAM_START\n[AI Response]: ");
            if (data != nullptr && len > 0) {
                Serial.write(data, len);
            }
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
            if (gNeedReset == 1) {
                PR_INFO("Device Reset!");
                tal_system_reset();
            }
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
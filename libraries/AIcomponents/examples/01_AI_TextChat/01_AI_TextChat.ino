/**
 * @file AI_TextChat.ino
 * @brief AI Text Chat - simple text-based AI interaction via Serial
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 * @note Only supports TUYA_T5AI platform
 */
#include <Arduino.h>
#include "TuyaAI.h"
#include "TuyaIoT.h"
#include "Log.h"

/***********************************************************
************************macro define************************
***********************************************************/
// Device credentials (replace with your own)
#define TUYA_DEVICE_UUID    "uuidxxxxxxxxxxxxxxxx"
#define TUYA_DEVICE_AUTHKEY "keyxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
#define TUYA_PRODUCT_ID     "9inb01mvjqh5zhhr"

#define LED_PIN             1

#define DPID_VOLUME         3
/***********************************************************
***********************static declarations******************
***********************************************************/
static void aiEventCallback(AI_USER_EVT_TYPE_E event, uint8_t *data, uint32_t len, void *arg);
static void tuyaIoTEventCallback(tuya_event_msg_t *event);
static void handleUserInput(void);
/***********************************************************
***********************public functions*********************
***********************************************************/
void setup()
{
    Serial.begin(115200);
    Log.begin();
    Log.setLevel(LogClass::WARN);
    
    // Print startup banner
    PR_NOTICE("============ Tuya AI Text Chat ==============");
    PR_NOTICE("Compile time:        %s", __DATE__);

    board_register_hardware();
    pinMode(LED_PIN, OUTPUT);
    
    // TuyaIoT setup
    TuyaIoT.resetNetcfg();
    TuyaIoT.setEventCallback(tuyaIoTEventCallback);
    TuyaIoT.setLicense(TUYA_DEVICE_UUID, TUYA_DEVICE_AUTHKEY);
    TuyaIoT.begin(TUYA_PRODUCT_ID, PROJECT_VERSION);

    // TuyaAI setup
    AIConfig_t cfg = {AI_CHAT_MODE_WAKEUP, 70, aiEventCallback, nullptr, nullptr};
    TuyaAI.begin(cfg);
    TuyaAI.Audio.begin();

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
static void aiEventCallback(AI_USER_EVT_TYPE_E event, uint8_t *data, uint32_t len, void *arg)
{
    switch (event) {
        case AI_USER_EVT_TEXT_STREAM_START:
            Serial.print("[AI]: ");
            if (data && len > 0) Serial.write(data, len);
            break;
        case AI_USER_EVT_TEXT_STREAM_DATA:
            if (data && len > 0) Serial.write(data, len);
            break;
        case AI_USER_EVT_TEXT_STREAM_STOP:
            Serial.println("\n");
            break;
        default:
            break;
    }
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
            Serial.println("Device MQTT Connected!");
            tal_event_publish(EVENT_MQTT_CONNECTED, NULL);
            break;
            
        case TUYA_EVENT_MQTT_DISCONNECT:
            PR_INFO("Device MQTT Disconnected!");
            tal_event_publish(EVENT_MQTT_DISCONNECTED, NULL);
            break;

        case TUYA_EVENT_TIMESTAMP_SYNC:
            PR_INFO("Sync timestamp:%d", event->value.asInteger);
            tal_time_set_posix(event->value.asInteger, 1);
            break;
            
        default:
            break;
    }
}

static void handleUserInput(void) {
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

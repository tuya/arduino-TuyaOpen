/**
 * @file 08_AI_PictureGenerate.ino
 * @brief AI Picture Generation - ask the AI to draw, then store and show the result
 *
 * Flow:
 * 1. TuyaAI.Picture.setOutputSize() tells the cloud what resolution to render at.
 * 2. A prompt is sent as ordinary text ("draw a cat wearing a hat").
 * 3. The generated picture is written into the on-device album by the AI
 *    picture component, which raises AI_USER_EVT_GENERATE_PICTURE carrying the
 *    album filename.
 * 4. The album page is opened on the display so the result is visible.
 *
 * AI_USER_EVT_GET_PICTURE_FROM_APP is raised the same way when a picture is
 * pushed from the Tuya app instead of being generated.
 *
 * Vendor build requirement:
 * - CONFIG_ENABLE_COMP_AI_PICTURE=y
 * - A display, for the album page (the T5AI board's 3.5" LCD by default)
 *
 * @copyright Copyright (c) 2021-2026 Tuya Inc. All Rights Reserved.
 *
 * @note ===================== Only supports TUYA_T5AI platform =====================
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

// Resolution the AI should render at
#define OUTPUT_WIDTH  320
#define OUTPUT_HEIGHT 480

/***********************************************************
***********************static declarations******************
***********************************************************/
static void aiEventCallback(AI_USER_EVT_TYPE_E event, uint8_t *data, uint32_t len, void *arg);
static void tuyaIoTEventCallback(tuya_event_msg_t *event);
static void handleUserInput(void);

static bool sgReady = false;

/***********************************************************
***********************public functions*********************
***********************************************************/
void setup()
{
    Serial.begin(115200);
    Log.begin();
    Log.setLevel(LogClass::WARN);

    PR_NOTICE("============ Tuya AI Picture Generation ==============");
    PR_NOTICE("Compile time:        %s", __DATE__);

    board_register_hardware();

    // TuyaIoT setup - picture generation runs in the cloud, so the device has
    // to be provisioned and connected first
    TuyaIoT.resetNetcfg();
    TuyaIoT.setEventCallback(tuyaIoTEventCallback);
    TuyaIoT.setLicense(TUYA_DEVICE_UUID, TUYA_DEVICE_AUTHKEY);
    TuyaIoT.begin(TUYA_PRODUCT_ID, PROJECT_VERSION);

    // TuyaAI setup, with the WeChat-style UI that owns the album pages
    AIConfig_t cfg = {AI_CHAT_MODE_WAKEUP, 70, aiEventCallback, nullptr, nullptr};
    TuyaAI.begin(cfg);
    TuyaAI.Audio.begin();
    TuyaAI.UI.begin(BOT_UI_WECHAT);

    if (TuyaAI.Picture.begin() != OPRT_OK) {
        PR_ERR("Album init failed");
    }
    TuyaAI.Picture.setOutputSize(OUTPUT_WIDTH, OUTPUT_HEIGHT);

    delay(2000);
    TuyaIoT.resetNetconfigCheck();

    Serial.println("Type a prompt and press Enter, or 'v' to browse the album.");
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
    // Both events carry the album filename of the stored picture
    case AI_USER_EVT_GENERATE_PICTURE:
    case AI_USER_EVT_GET_PICTURE_FROM_APP:
        if (data && len > 0) {
            Serial.print("\n[Picture stored as]: ");
            Serial.write(data, len);
            Serial.println();
        }
        // Show it: open the album view page on the display
        TuyaAI.UI.displayMessage(AI_UI_DISP_ALBUM_OPEN, nullptr, 0);
        break;

    case AI_USER_EVT_TEXT_STREAM_START:
        Serial.print("[AI]: ");
        if (data && len > 0) {
            Serial.write(data, len);
        }
        break;

    case AI_USER_EVT_TEXT_STREAM_DATA:
        if (data && len > 0) {
            Serial.write(data, len);
        }
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
        sgReady = true;
        tal_event_publish(EVENT_MQTT_CONNECTED, NULL);
        break;

    case TUYA_EVENT_MQTT_DISCONNECT:
        PR_INFO("Device MQTT Disconnected!");
        sgReady = false;
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

/**
 * @brief Send whatever is typed as a drawing prompt
 */
static void handleUserInput(void)
{
    static int     i = 0;
    static uint8_t buf[256];

    while (Serial.available()) {
        char c = Serial.read();

        if (i == 0 && (c == 'v' || c == 'V')) {
            TuyaAI.UI.displayMessage(AI_UI_DISP_ALBUM_VIEW_ALL, nullptr, 0);
            continue;
        }

        if (i < (int)sizeof(buf) - 1) {
            buf[i++] = (uint8_t)c;
        }

        if (c == '\n' || c == '\r') {
            if (!sgReady) {
                Serial.println("Device is not connected yet, please finish provisioning first.");
            } else {
                Serial.print("\n[Prompt]: ");
                Serial.write(buf, i);
                TuyaAI.sendText(buf, i);
            }
            i = 0;
            memset(buf, 0, sizeof(buf));
        }
    }
}

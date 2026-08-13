/**
 * @file 07_AI_PictureInput.ino
 * @brief AI Picture Input - let the AI look at a picture and answer questions about it
 *
 * Two ways to send a picture to the AI are shown:
 *
 * 1. Direct recognition - hand a JPEG buffer straight to the AI with
 *    TuyaAI.Picture.recognize(). Nothing is stored on the device.
 * 2. Album attachment  - save the picture into the on-device album, queue it
 *    with TuyaAI.Picture.attachFromAlbum(name, question), then release the batch
 *    with TuyaAI.Picture.sendAttachments(). Several pictures can be queued.
 *
 * The picture here comes from the camera via TuyaAI.Video, whose live preview
 * page is put on the display with AI_UI_DISP_CAMERA_OPEN. The AI answers over
 * the normal text stream events, so this example also prints the reply.
 *
 * Vendor build requirement:
 * - CONFIG_ENABLE_COMP_AI_PICTURE=y (album + picture input/output)
 * - CONFIG_ENABLE_COMP_AI_VIDEO=y   (camera capture)
 *
 * @copyright Copyright (c) 2021-2026 Tuya Inc. All Rights Reserved.
 *
 * @note ===================== Only supports TUYA_T5AI platform with a camera =====================
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

#define QUESTION "What do you see in this picture?"

/***********************************************************
***********************static declarations******************
***********************************************************/
static void aiEventCallback(AI_USER_EVT_TYPE_E event, uint8_t *data, uint32_t len, void *arg);
static void tuyaIoTEventCallback(tuya_event_msg_t *event);
static void handleUserInput(void);
static void askAboutSnapshot(bool viaAlbum);

static bool sgReady = false;

/***********************************************************
***********************public functions*********************
***********************************************************/
void setup()
{
    Serial.begin(115200);
    Log.begin();
    Log.setLevel(LogClass::WARN);

    PR_NOTICE("============ Tuya AI Picture Input ==============");
    PR_NOTICE("Compile time:        %s", __DATE__);

    board_register_hardware();

    // TuyaIoT setup - the AI components only work once the device is
    // provisioned and connected, exactly as in the other AI examples
    TuyaIoT.resetNetcfg();
    TuyaIoT.setEventCallback(tuyaIoTEventCallback);
    TuyaIoT.setLicense(TUYA_DEVICE_UUID, TUYA_DEVICE_AUTHKEY);
    TuyaIoT.begin(TUYA_PRODUCT_ID, PROJECT_VERSION);

    // TuyaAI setup
    AIConfig_t cfg = {AI_CHAT_MODE_WAKEUP, 70, aiEventCallback, nullptr, nullptr};
    TuyaAI.begin(cfg);
    TuyaAI.Audio.begin();
    TuyaAI.UI.begin(BOT_UI_WECHAT);

    // Camera + album
    if (TuyaAI.Video.begin() != OPRT_OK || TuyaAI.Video.start() != OPRT_OK) {
        PR_ERR("Camera init failed");
    }
    if (TuyaAI.Picture.begin() != OPRT_OK) {
        PR_ERR("Album init failed");
    }
    PR_NOTICE("Album name: %s", TuyaAI.Picture.albumName());

    // Show the live camera page so the framing is visible before capturing
    TuyaAI.UI.displayMessage(AI_UI_DISP_CAMERA_OPEN, nullptr, 0);

    delay(2000);
    TuyaIoT.resetNetconfigCheck();

    Serial.println("Type 'r' to ask about a snapshot, 'a' to ask via the album, 'x' to close the preview.");
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
 * @brief Grab one camera frame and send it to the AI
 * @param viaAlbum true  -> save into the album, attach it with a question
 *                 false -> recognize the JPEG directly
 */
static void askAboutSnapshot(bool viaAlbum)
{
    if (!sgReady) {
        Serial.println("Device is not connected yet, please finish provisioning first.");
        return;
    }

    uint8_t *jpeg = nullptr;
    uint32_t len  = 0;

    if (TuyaAI.Video.getJpegFrame(&jpeg, &len) != OPRT_OK || jpeg == nullptr || len == 0) {
        PR_ERR("No camera frame available");
        return;
    }

    if (viaAlbum) {
        char name[AI_PICTURE_NAME_MAX_LEN + 1] = {0};

        if (TuyaAI.Picture.saveToAlbum(jpeg, len, nullptr, name, sizeof(name)) == OPRT_OK) {
            PR_NOTICE("Saved as %s, asking: %s", name, QUESTION);
            TuyaAI.Picture.attachFromAlbum(name, QUESTION);
            TuyaAI.Picture.sendAttachments();
        } else {
            PR_ERR("Failed to save the picture into the album");
        }
    } else {
        PR_NOTICE("Sending %u bytes for recognition", (unsigned int)len);
        TuyaAI.Picture.recognize(jpeg, len);
    }

    TuyaAI.Video.freeJpegFrame(&jpeg);
}

static void aiEventCallback(AI_USER_EVT_TYPE_E event, uint8_t *data, uint32_t len, void *arg)
{
    switch (event) {
    case AI_USER_EVT_SEND_PICTURE_END:
        Serial.println("[Picture sent, waiting for the answer]");
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

static void handleUserInput(void)
{
    while (Serial.available()) {
        char c = Serial.read();

        if (c == 'r' || c == 'R') {
            askAboutSnapshot(false);
        } else if (c == 'a' || c == 'A') {
            askAboutSnapshot(true);
        } else if (c == 'x' || c == 'X') {
            TuyaAI.UI.displayMessage(AI_UI_DISP_CAMERA_CLOSE, nullptr, 0);
        }
    }
}

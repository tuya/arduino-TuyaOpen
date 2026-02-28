/**
 * @file dpType.ino
 * @brief Tuya IoT DP type example
 * @copyright Copyright (c) 2021-2026 Tuya Inc. All Rights Reserved.
 */

#include "TuyaIoT.h"
#include "Log.h"

// button
#define buttonPin         BUTTON_BUILTIN
#define buttonPressLevel  LOW
#define buttonDebounceMs  (50u)
#define buttonLongPressMs (3 * 1000u)

#define DPID_SWITCH 20
#define DPID_MODE   21
#define DPID_BRIGHT 22
#define DPID_BITMAP 101
#define DPID_STRING 102
#define DPID_RAW    103

// Tuya license
#define TUYA_DEVICE_UUID    "uuidxxxxxxxxxxxxxxxx"
#define TUYA_DEVICE_AUTHKEY "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"

void tuyaIoTEventCallback(tuya_event_msg_t *event);
void buttonCheck(void);

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);

    Log.begin();

    // button
    pinMode(buttonPin, INPUT_PULLUP);

    TuyaIoT.setEventCallback(tuyaIoTEventCallback);

    // license
    tuya_iot_license_t license;
    int                rt = TuyaIoT.readBoardLicense(&license);
    if (OPRT_OK != rt) {
        license.uuid    = (char *)TUYA_DEVICE_UUID;
        license.authkey = (char *)TUYA_DEVICE_AUTHKEY;
        Serial.println("Replace the TUYA_DEVICE_UUID and TUYA_DEVICE_AUTHKEY contents, otherwise the demo cannot work");
    }
    Serial.print("uuid: ");
    Serial.println(license.uuid);
    Serial.print("authkey: ");
    Serial.println(license.authkey);
    TuyaIoT.setLicense(license.uuid, license.authkey);

    // The "PROJECT_VERSION" comes from the "PROJECT_VERSION" field in "appConfig.json"
    TuyaIoT.begin("2avicuxv6zgeiquf", PROJECT_VERSION);
}

void loop()
{
    // put your main code here, to run repeatedly:

    // button press check
    buttonCheck();

    delay(10);
}

void tuyaIoTEventCallback(tuya_event_msg_t *event)
{
    tuya_event_id_t event_id = TuyaIoT.eventGetId(event);

    switch (event_id) {
    case TUYA_EVENT_BIND_START: {
        PR_DEBUG("TUYA_EVENT_BIND_START");
    } break;
    case TUYA_EVENT_ACTIVATE_SUCCESSED: {
        PR_DEBUG("TUYA_EVENT_ACTIVATE_SUCCESSED");
    } break;
    case TUYA_EVENT_TIMESTAMP_SYNC: {
        tal_time_set_posix(event->value.asInteger, 1);
    } break;
    case TUYA_EVENT_DP_RECEIVE_OBJ: {
        uint16_t dpNum = TuyaIoT.eventGetDpNum(event);
        for (uint16_t i = 0; i < dpNum; i++) {
            uint8_t dpid = TuyaIoT.eventGetDpId(event, i);
            switch (dpid) {
            case DPID_SWITCH: {
                bool switchStatus = 0;
                TuyaIoT.read(event, DPID_SWITCH, switchStatus);
                TuyaIoT.write(DPID_SWITCH, switchStatus);
                PR_DEBUG("switchStatus: %d", switchStatus);
            } break;
            case DPID_MODE: {
                uint32_t mode = 0;
                TuyaIoT.read(event, DPID_MODE, mode);
                TuyaIoT.write(DPID_MODE, mode);
                PR_DEBUG("mode: %d", mode);
            } break;
            case DPID_BRIGHT: {
                int brightValue = 0;
                TuyaIoT.read(event, DPID_BRIGHT, brightValue);
                TuyaIoT.write(DPID_BRIGHT, brightValue);
                PR_DEBUG("brightValue: %d", brightValue);
            } break;
            case DPID_STRING: {
                char *strValue = NULL;
                TuyaIoT.read(event, DPID_STRING, strValue);
                TuyaIoT.write(DPID_STRING, strValue);
            } break;
            default:
                break;
            }
        }
    } break;
    case TUYA_EVENT_DP_RECEIVE_RAW: {
        uint8_t *rawValue = NULL;
        uint16_t len      = 0;
        TuyaIoT.read(event, DPID_RAW, rawValue, len);
        PR_DEBUG("---> len:%d", len);
        PR_HEXDUMP_DEBUG("raw", rawValue, len);
        TuyaIoT.write(DPID_RAW, rawValue, len);
    } break;
    default:
        break;
    }
}

void buttonClick()
{
    Serial.println("Button clicked");
}

void buttonLongPressStart()
{
    Serial.println("Button long press, remove Tuya IoT device.");
    TuyaIoT.remove();
}

void buttonCheck(void)
{
    static uint32_t buttonPressMs = 0;
    static uint8_t  isPress       = 0;

    if (digitalRead(buttonPin) == buttonPressLevel) {
        if (isPress == 0) {
            buttonPressMs = millis();
            isPress       = 1;
        }

        // button debounce
        if ((1 == isPress) && ((millis() - buttonPressMs) > buttonDebounceMs)) {
            isPress = 2;
        }

        // long press check
        if ((2 == isPress) && ((millis() - buttonPressMs) >= buttonLongPressMs)) {
            isPress = 3;
            buttonLongPressStart();
        }
    } else {
        if (isPress == 2) {
            if ((millis() - buttonPressMs) < buttonLongPressMs) {
                buttonClick();
            } else {
                buttonLongPressStart();
            }
        }
        isPress = 0;
    }
}

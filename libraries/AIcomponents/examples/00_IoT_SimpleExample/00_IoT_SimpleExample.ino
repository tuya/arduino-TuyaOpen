/**
 * @file IoT_SimpleExample.ino
 * @brief A simple example demonstrating Tuya IoT switch functionality
 *
 * This source file provides a basic implementation of a switch device using the Tuya IoT platform.
 * It includes functionality for device initialization, event handling, and network communication.
 * This example is intended for developers looking to create IoT applications that require switch capabilities
 * and integration with the Tuya IoT ecosystem.
 *
 * @copyright Copyright (c) 2021-2026 Tuya Inc. All Rights Reserved.
 */
#include "Log.h"
#include "TuyaIoT.h"

/***********************************************************
************************macro define************************
***********************************************************/
#define TUYA_DEVICE_UUID    "uuidxxxxxxxxxxxxxxxx"
#define TUYA_DEVICE_AUTHKEY "keyxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
#define TUYA_PRODUCT_ID     "qhivvyqawogv04e4"

#define APP_LED_PIN 2
/***********************************************************
***********************function define**********************
***********************************************************/
void tuyaIoTEventCallback(tuya_event_msg_t *event);

void setup(void)
{
    Serial.begin(115200);
    Log.begin(1024);

    // Print startup banner
    PR_NOTICE("============ Tuya IoT Simple Example ==============");
    PR_NOTICE("Compile time:        %s", __DATE__);

    pinMode(APP_LED_PIN, OUTPUT);
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
    license.uuid    = (char *)TUYA_DEVICE_UUID;
    license.authkey = (char *)TUYA_DEVICE_AUTHKEY;
    TuyaIoT.setLicense(license.uuid, license.authkey);

    // The "PROJECT_VERSION" comes from the "PROJECT_VERSION" field in "appConfig.json"
    TuyaIoT.begin(TUYA_PRODUCT_ID, PROJECT_VERSION);
}

void loop()
{
    while (Serial.available() > 0) {
        char c = Serial.read();
        Serial.write(&c, 1);
    }
    delay(100);
}

void tuyaIoTEventCallback(tuya_event_msg_t *event)
{
    tuya_event_id_t event_id = TuyaIoT.eventGetId(event);

    PR_DEBUG("Tuya Event ID:%d(%s)", event_id, EVENT_ID2STR(event_id));
    PR_INFO("Device Free heap %d", tal_system_get_free_heap_size());
    switch (event_id) {
    case TUYA_EVENT_BIND_START:
        PR_INFO("Device Bind Start!");
        break;

    /* Print the QRCode for Tuya APP bind */
    case TUYA_EVENT_DIRECT_MQTT_CONNECTED:
        break;

    /* MQTT with tuya cloud is connected, device online */
    case TUYA_EVENT_MQTT_CONNECTED:
        PR_INFO("Device MQTT Connected!");
        break;

    /* RECV upgrade request */
    case TUYA_EVENT_UPGRADE_NOTIFY:
        break;

    /* Sync time with tuya Cloud */
    case TUYA_EVENT_TIMESTAMP_SYNC:
        PR_INFO("Sync timestamp:%d", event->value.asInteger);
        tal_time_set_posix(event->value.asInteger, 1);
        break;

    case TUYA_EVENT_RESET:
        PR_INFO("Device Reset:%d", event->value.asInteger);
        break;

    /* RECV OBJ DP */
    case TUYA_EVENT_DP_RECEIVE_OBJ: {
        dp_obj_recv_t *dpobj = event->value.dpobj;
        PR_DEBUG("SOC Rev DP Cmd t1:%d t2:%d CNT:%u", dpobj->cmd_tp, dpobj->dtt_tp, dpobj->dpscnt);
        if (dpobj->devid != NULL) {
            PR_DEBUG("devid.%s", dpobj->devid);
        }

        uint32_t index = 0;
        for (index = 0; index < dpobj->dpscnt; index++) {
            dp_obj_t *dp = dpobj->dps + index;
            PR_DEBUG("idx:%d dpid:%d type:%d ts:%u", index, dp->id, dp->type, dp->time_stamp);
            switch (dp->type) {
            case PROP_BOOL: {
                PR_DEBUG("bool value:%d", dp->value.dp_bool);
                if (dp->value.dp_bool == true) {
                    digitalWrite(APP_LED_PIN, LOW);
                } else {
                    digitalWrite(APP_LED_PIN, HIGH);
                }
                break;
            }
            case PROP_VALUE: {
                PR_DEBUG("int value:%d", dp->value.dp_value);
                break;
            }
            case PROP_STR: {
                PR_DEBUG("str value:%s", dp->value.dp_str);
                break;
            }
            case PROP_ENUM: {
                PR_DEBUG("enum value:%u", dp->value.dp_enum);
                break;
            }
            case PROP_BITMAP: {
                PR_DEBUG("bits value:0x%X", dp->value.dp_bitmap);
                break;
            }
            default: {
                PR_ERR("idx:%d dpid:%d type:%d ts:%u is invalid", index, dp->id, dp->type, dp->time_stamp);
                break;
            }
            } // end of switch
        }

        TuyaIoT.write((dpobj->dps->id), (dpobj->dps->value), 0);

    } break;

    /* RECV RAW DP */
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

        /* TBD.. add other event if necessary */

    default:
        break;
    }
}

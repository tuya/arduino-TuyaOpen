/**
 * @file TuyaIoT.cpp
 * @brief Tuya IoT Arduino library implementation
 * @copyright Copyright (c) 2021-2026 Tuya Inc. All Rights Reserved.
 */

/******************************************************************************
 * INCLUDE
 ******************************************************************************/
#include "TuyaIoT.h"
#include "ArduinoTuyaIoTClient.h"

extern "C" {
#include <string.h>

#include "tuya_iot_dp.h"
#include "tal_time_service.h"
#include "tal_cli.h"
#if defined(ARDUINO_CHIP_T5) || defined(ARDUINO_CHIP_esp32)
#include "tuya_authorize.h"
#endif
#include "reset_netcfg.h"
}

/******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/******************************************************************************
 * LOCAL MODULE FUNCTIONS
 ******************************************************************************/

/******************************************************************************
 * CTOR/DTOR
 ******************************************************************************/
TuyaIoTCloudClass::TuyaIoTCloudClass()
{
    ;
}

TuyaIoTCloudClass::~TuyaIoTCloudClass()
{
    ;
}

/******************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/
void TuyaIoTCloudClass::setEventCallback(void (*callback)(tuya_event_msg_t *event))
{
    app_iot_event_register_cb(callback);
}

void TuyaIoTCloudClass::setLicense(const char *uuid, const char *authKey)
{
    strncpy(this->_uuid, uuid, MAX_LENGTH_UUID);
    strncpy(this->_authKey, authKey, MAX_LENGTH_AUTHKEY);
    app_iot_license_set(this->_uuid, this->_authKey);
}

void TuyaIoTCloudClass::setLogLevel(TAL_LOG_LEVEL_E level)
{
    tal_log_set_level(level);
}

void TuyaIoTCloudClass::begin(const char *pid, const char *version)
{
    PR_DEBUG("TuyaIoTCloudClass::begin pid=%s, version=%s", pid, version);
    strncpy(this->_pid, pid, MAX_LENGTH_PRODUCT_ID);
    strncpy(this->_version, version, MAX_LENGTH_SW_VER);

    app_iot_task_start(this->_pid, this->_version);
}

void TuyaIoTCloudClass::begin(const char *pid, const char *version, const char *uuid, const char *authKey)
{
    strncpy(this->_pid, pid, MAX_LENGTH_PRODUCT_ID);
    strncpy(this->_version, version, MAX_LENGTH_SW_VER);
    strncpy(this->_uuid, uuid, MAX_LENGTH_UUID);
    strncpy(this->_authKey, authKey, MAX_LENGTH_AUTHKEY);

    app_iot_license_set(this->_uuid, this->_authKey);
    app_iot_task_start(this->_pid, this->_version);
}

int TuyaIoTCloudClass::objWrite(uint8_t dpid, void *value, TIME_T timestamp)
{
    dp_obj_t dpObj;

    memset(&dpObj, 0, sizeof(dp_obj_t));

    dp_node_t *dpNode = dp_node_find_by_devid(ArduinoIoTClient.activate.devid, dpid);
    if (dpNode == NULL) {
        PR_ERR("dpid %d not found", dpid);
        return OPRT_COM_ERROR;
    }

    if (T_OBJ != dpNode->desc.type) {
        PR_ERR("dpid %d not object type", dpid);
        return OPRT_NOT_SUPPORTED;
    }

    // dpid
    dpObj.id = dpid;
    // type
    dpObj.type = dpNode->desc.prop_tp;
    // timestamp
    dpObj.time_stamp = timestamp;
    // value
    switch (dpNode->desc.prop_tp) {
    case (PROP_STR): {
        dpObj.value.dp_str = reinterpret_cast<char *>(value);
    } break;
    case PROP_VALUE: {
        dpObj.value.dp_value = *static_cast<int *>(value);
    } break;
    case PROP_ENUM: {
        dpObj.value.dp_enum = *static_cast<uint32_t *>(value);
    } break;
    case PROP_BITMAP: {
        dpObj.value.dp_bitmap = *static_cast<uint32_t *>(value);
    } break;
    case PROP_BOOL: {
        dpObj.value.dp_bool = *static_cast<bool *>(value);
    } break;
    default: {
        PR_ERR("The data type (void *) is not supported, dpid %d type %d  ", dpid, dpNode->desc.prop_tp);
        return OPRT_COM_ERROR;
    }
    }

    return tuya_iot_dp_obj_report(&ArduinoIoTClient, ArduinoIoTClient.activate.devid, &dpObj, 1, timestamp);
    ;
}

int TuyaIoTCloudClass::rawWrite(uint8_t dpid, uint8_t *value, uint16_t len, uint32_t timeout)
{
    int rt = OPRT_OK;

    dp_raw_t *dpRaw      = NULL;
    size_t    mallocSize = 0;

    PR_DEBUG("rawWrite");

    dp_node_t *dpNode = dp_node_find_by_devid(ArduinoIoTClient.activate.devid, dpid);
    if (dpNode == NULL) {
        PR_ERR("dpid %d not found", dpid);
        return OPRT_COM_ERROR;
    }

    if (T_RAW != dpNode->desc.type) {
        PR_ERR("dpid %d not raw type", dpid);
        return OPRT_NOT_SUPPORTED;
    }

    mallocSize = sizeof(dp_raw_t) + len;
    dpRaw      = (dp_raw_t *)tal_malloc(mallocSize);
    if (dpRaw == NULL) {
        PR_ERR("malloc failed");
        return OPRT_MALLOC_FAILED;
    }
    memset(dpRaw, 0, mallocSize);

    PR_HEXDUMP_DEBUG("rawWrite", value, len);

    dpRaw->id  = dpid;
    dpRaw->len = len;
    memcpy(dpRaw->data, value, len);

    rt = tuya_iot_dp_raw_report(&ArduinoIoTClient, ArduinoIoTClient.activate.devid, dpRaw, timeout);

    tal_free(dpRaw);
    dpRaw = NULL;

    return rt;
}

int TuyaIoTCloudClass::objRead(tuya_event_msg_t *event, uint8_t dpid, dp_prop_tp_t &type, dp_value_t &value)
{
    int rt = OPRT_OK;

    if (NULL == event) {
        PR_ERR("event is NULL");
        return OPRT_INVALID_PARM;
    }

    if (TUYA_EVENT_DP_RECEIVE_OBJ != event->id) {
        PR_ERR("event id is not TUYA_EVENT_DP_RECEIVE_OBJ");
        return OPRT_NOT_SUPPORTED;
    }

    dp_obj_recv_t *dpObj = event->value.dpobj;
    dp_obj_t      *p     = NULL;

    // find dp
    for (int i = 0; i < dpObj->dpscnt; i++) {
        if (dpid == dpObj->dps[i].id) {
            p = &dpObj->dps[i];
            break;
        }
    }

    if (!p) {
        PR_ERR("Not find dp: %d", dpid);
        return OPRT_COM_ERROR;
    }

    type  = p->type;
    value = p->value;

    return rt;
}

int TuyaIoTCloudClass::read(tuya_event_msg_t *event, uint8_t dpid, uint8_t *&value, uint16_t &len)
{
    int rt = OPRT_OK;

    if (NULL == event) {
        PR_ERR("event is NULL");
        return OPRT_INVALID_PARM;
    }

    if (TUYA_EVENT_DP_RECEIVE_RAW != event->id) {
        PR_ERR("event id is not TUYA_EVENT_DP_RECEIVE_RAW");
        return OPRT_NOT_SUPPORTED;
    }

    dp_raw_recv_t *dpraw = event->value.dpraw;
    if (NULL == dpraw) {
        PR_ERR("dpraw is null");
        return OPRT_COM_ERROR;
    }

    if (dpid != dpraw->dp.id) {
        PR_ERR("dpid not match");
        return OPRT_INVALID_PARM;
    }

    value = dpraw->dp.data;
    len   = dpraw->dp.len;

    return rt;
}

tuya_event_id_t TuyaIoTCloudClass::eventGetId(tuya_event_msg_t *event)
{
    return event->id;
}

uint16_t TuyaIoTCloudClass::eventGetDpNum(tuya_event_msg_t *event)
{
    if (NULL == event) {
        PR_ERR("event is NULL");
        return 0;
    }

    if (TUYA_EVENT_DP_RECEIVE_OBJ != event->id) {
        PR_ERR("event id is not TUYA_EVENT_DP_RECEIVE_OBJ");
        return 0;
    }

    return event->value.dpobj->dpscnt;
}

uint8_t TuyaIoTCloudClass::eventGetDpId(tuya_event_msg_t *event, uint16_t index)
{
    if (NULL == event) {
        PR_ERR("event is NULL");
        return 0;
    }

    if (TUYA_EVENT_DP_RECEIVE_OBJ != event->id) {
        PR_ERR("event id is not TUYA_EVENT_DP_RECEIVE_OBJ");
        return 0;
    }

    if (event->value.dpobj == NULL) {
        PR_ERR("dpobj is NULL");
        return 0;
    }

    if (index >= event->value.dpobj->dpscnt) {
        PR_ERR("dp index out of range, index=%u dpscnt=%u", index, event->value.dpobj->dpscnt);
        return 0;
    }

    return event->value.dpobj->dps[index].id;
}

int TuyaIoTCloudClass::remove(void)
{
    return tuya_iot_reset(&ArduinoIoTClient);
}

int TuyaIoTCloudClass::readBoardLicense(tuya_iot_license_t *license)
{
    return tuya_iot_license_read(license);
}

bool TuyaIoTCloudClass::networkCheck(void)
{
    if (ArduinoIoTClient.config.network_check && ArduinoIoTClient.config.network_check()) {
        return true;
    }

    return false;
}

bool TuyaIoTCloudClass::isActivated(void)
{
    return tuya_iot_activated(&ArduinoIoTClient);
}

bool TuyaIoTCloudClass::isTimeSync(void)
{
    int rt = OPRT_OK;

    rt = tal_time_check_time_sync();

    return (OPRT_OK == rt) ? (true) : (false);
}

void TuyaIoTCloudClass::uartAuthInit()
{
    tal_cli_init();
#if defined(ARDUINO_CHIP_T5) || defined(ARDUINO_CHIP_esp32)
    tuya_authorize_init();
#endif
}

void TuyaIoTCloudClass::resetNetcfg()
{
    reset_netconfig_start();
}

int TuyaIoTCloudClass::resetNetconfigCheck(void)
{
    return reset_netconfig_check();
}
/******************************************************************************
 * PRIVATE MEMBER FUNCTIONS
 ******************************************************************************/

/******************************************************************************
 * EXTERN DEFINITION
 ******************************************************************************/
TuyaIoTCloudClass TuyaIoT;

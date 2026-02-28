/**
 * @file ArduinoTuyaIoTClient.c
 * @brief Arduino Tuya IoT client implementation
 * @copyright Copyright (c) 2021-2026 Tuya Inc. All Rights Reserved.
 */

/******************************************************************************
 * INCLUDE
 ******************************************************************************/
#include <Arduino.h>

#include "ArduinoTuyaIoTClient.h"

#include "netmgr.h"
#include "tal_api.h"
#include "tuya_iot.h"
#include "tuya_iot_dp.h"
#include <assert.h>

#if defined(ENABLE_WIFI) && (ENABLE_WIFI == 1)
#include "netconn_wifi.h"
#endif
#if defined(ENABLE_WIRED) && (ENABLE_WIRED == 1)
#include "netconn_wired.h"
#endif

/******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/******************************************************************************
 * TYPEDEF
 ******************************************************************************/
typedef void (*EvevntCallbackFunc_T)(tuya_event_msg_t *event);

/******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/
static const char *sg_softwareVersion = NULL;
static const char *sg_productID       = NULL;
static const char *sg_uuid            = NULL;
static const char *sg_authKey         = NULL;

static EvevntCallbackFunc_T eventCallback = NULL;

static THREAD_HANDLE iot_thread = NULL;

/* Tuya device handle */
tuya_iot_client_t ArduinoIoTClient;

/******************************************************************************
 * LOCAL MODULE FUNCTIONS
 ******************************************************************************/
static void user_event_handler_on(tuya_iot_client_t *client, tuya_event_msg_t *event);
static bool user_network_check(void);
static void tuya_app_thread(void *arg);

/******************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/
void app_iot_event_register_cb(void (*callback)(tuya_event_msg_t *event))
{
    eventCallback = callback;
}

void app_iot_license_set(const char *uuid, const char *authKey)
{
    sg_uuid    = uuid;
    sg_authKey = authKey;
    return;
}

void app_iot_task_start(const char *pid, const char *version)
{
    sg_softwareVersion = version;
    sg_productID       = pid;

    THREAD_CFG_T thrd_param = {8 * 1024, THREAD_PRIO_1, "iot_task"};
    tal_thread_create_and_start(&iot_thread, NULL, NULL, tuya_app_thread, NULL, &thrd_param);
    return;
}

/******************************************************************************
 * PRIVATE MEMBER FUNCTIONS
 ******************************************************************************/
static void user_event_handler_on(tuya_iot_client_t *client, tuya_event_msg_t *event)
{
    PR_DEBUG("Tuya Event ID:%d(%s)", event->id, EVENT_ID2STR(event->id));
    PR_INFO("Device Free heap %d", tal_system_get_free_heap_size());

    if (eventCallback != NULL) {
        eventCallback(event);
    }
}

static bool user_network_check(void)
{
    netmgr_status_e status = NETMGR_LINK_DOWN;
    netmgr_conn_get(NETCONN_AUTO, NETCONN_CMD_STATUS, &status);
    return (status == NETMGR_LINK_DOWN) ? false : true;
}

static void tuya_app_thread(void *arg)
{
    int rt = OPRT_OK;

    tuya_iot_config_t iot_cfg;
    memset(&iot_cfg, 0, sizeof(tuya_iot_config_t));
    iot_cfg.software_ver  = sg_softwareVersion;
    iot_cfg.productkey    = sg_productID;
    iot_cfg.uuid          = sg_uuid;
    iot_cfg.authkey       = sg_authKey;
    iot_cfg.event_handler = user_event_handler_on;
    iot_cfg.network_check = user_network_check;

    /* Initialize Tuya device configuration */
    rt = tuya_iot_init(&ArduinoIoTClient, &iot_cfg);
    assert(rt == OPRT_OK);

    // network init
    netmgr_type_e type;
#if defined(ENABLE_WIFI) && (ENABLE_WIFI == 1)
    type = NETCONN_WIFI;
#endif
#if defined(ENABLE_WIRED) && (ENABLE_WIRED == 1)
    type |= NETCONN_WIRED;
#endif
    netmgr_init(type);
#if defined(ENABLE_WIFI) && (ENABLE_WIFI == 1)
    netcfg_args_t netcfg_args;
    netcfg_args.type = NETCFG_TUYA_BLE;
    netcfg_args.type |= NETCFG_TUYA_WIFI_AP;
    netmgr_conn_set(NETCONN_WIFI, NETCONN_CMD_NETCFG, &netcfg_args);
#endif

    /* Start tuya iot task */
    tuya_iot_start(&ArduinoIoTClient);

    for (;;) {
        tuya_iot_yield(&ArduinoIoTClient);
    }
}

/******************************************************************************
 * EXTERN DEFINITION
 ******************************************************************************/

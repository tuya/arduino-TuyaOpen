/**
 * @file appStatus.cpp
 * @brief Status monitoring and display update module
 *
 * This module handles periodic status updates including:
 * - WiFi signal strength monitoring using Arduino WiFi library
 * - Free heap size logging
 * - Network status display
 *
 * Display updates based on UI type:
 * - BOT_UI_USER: calls appDisplay functions
 * - Other types: calls TuyaAI.UI.displayMessage
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 */

#include "appStatus.h"
#include "appDisplay.h"
#include <WiFi.h>
#include "Log.h"
#include "lang_config.h"

/***********************************************************
************************macro define************************
***********************************************************/
#define PRINTF_FREE_HEAP_TIME       (5 * 1000)     // 5 seconds
#define DISP_NET_STATUS_TIME        (1 * 1000)      // 1 second

/***********************************************************
***********************variable define**********************
***********************************************************/
static bool gStatusReady = false;
static TIMER_ID gPrintHeapTimer = NULL;
static TIMER_ID gDispStatusTimer = NULL;
static AI_UI_WIFI_STATUS_E gLastWifiStatus = AI_UI_WIFI_STATUS_DISCONNECTED;

/***********************************************************
***********************static declarations******************
***********************************************************/
static void printHeapTimerCallback(TIMER_ID timer_id, void *arg);
static void dispStatusTimerCallback(TIMER_ID timer_id, void *arg);
static void updateWifiDisplay(AI_UI_WIFI_STATUS_E status);
static void updateStatusDisplay(const char *status);
static AI_UI_WIFI_STATUS_E getWifiStatus(void);
/***********************************************************
***********************public functions*********************
***********************************************************/
OPERATE_RET appStatusInit()
{
    OPERATE_RET rt = OPRT_OK;
    
    // Create and start heap print timer
    rt = tal_sw_timer_create(printHeapTimerCallback, NULL, &gPrintHeapTimer);
    if (rt != OPRT_OK) {
        PR_ERR("Failed to create heap timer: %d", rt);
        return OPRT_COM_ERROR;
    }
    tal_sw_timer_start(gPrintHeapTimer, PRINTF_FREE_HEAP_TIME, TAL_TIMER_CYCLE);
    
    // Display initial status
    updateWifiDisplay(gLastWifiStatus);
    updateStatusDisplay(INITIALIZING);
    
    // Create and start display status update timer
    rt = tal_sw_timer_create(dispStatusTimerCallback, NULL, &gDispStatusTimer);
    if (rt != OPRT_OK) {
        PR_ERR("Failed to create display status timer: %d", rt);
        return OPRT_COM_ERROR;
    }
    tal_sw_timer_start(gDispStatusTimer, DISP_NET_STATUS_TIME, TAL_TIMER_CYCLE);
    
    gStatusReady = true;
    
    return OPRT_OK;
}

void appStatusDeinit(void)
{
    if (gPrintHeapTimer) {
        tal_sw_timer_stop(gPrintHeapTimer);
        tal_sw_timer_delete(gPrintHeapTimer);
        gPrintHeapTimer = NULL;
    }
    
    if (gDispStatusTimer) {
        tal_sw_timer_stop(gDispStatusTimer);
        tal_sw_timer_delete(gDispStatusTimer);
        gDispStatusTimer = NULL;
    }
    
    gStatusReady = false;
    PR_NOTICE("Status monitoring deinitialized");
}

bool appStatusIsReady(void)
{
    return gStatusReady;
}

void appStatusUpdateWifi(AI_UI_WIFI_STATUS_E status)
{
    gLastWifiStatus = status;
    updateWifiDisplay(status);
}

void appStatusUpdateText(const char *status)
{
    updateStatusDisplay(status);
}

void appStatusUpdateMode(AI_CHAT_MODE_E mode)
{
    appDisplaySetMode((int)mode);
}

/***********************************************************
***********************timer callbacks**********************
***********************************************************/

/**
 * @brief Timer callback to print free heap size
 */
static void printHeapTimerCallback(TIMER_ID timer_id, void *arg)
{
    (void)timer_id;
    (void)arg;
    
#if defined(ENABLE_EXT_RAM) && (ENABLE_EXT_RAM == 1)
    uint32_t freeHeap = tal_system_get_free_heap_size();
    uint32_t freePsram = tal_psram_get_free_heap_size();
    PR_INFO("Free heap: %d, Free PSRAM: %d", freeHeap, freePsram);
#else
    uint32_t freeHeap = tal_system_get_free_heap_size();
    PR_INFO("Free heap: %d", freeHeap);
#endif
}

/**
 * @brief Timer callback to update display status
 */
static void dispStatusTimerCallback(TIMER_ID timer_id, void *arg)
{
    (void)timer_id;
    (void)arg;
    
    // Update WiFi status if changed
    AI_UI_WIFI_STATUS_E currentStatus = getWifiStatus();
    if (currentStatus != gLastWifiStatus) {
        gLastWifiStatus = currentStatus;
        updateWifiDisplay(currentStatus);
        PR_DEBUG("WiFi status changed: %d", currentStatus);
    }
}

/**
 * @brief Get current WiFi status based on RSSI using Arduino WiFi library
 * @return WiFi status enum
 */
static AI_UI_WIFI_STATUS_E getWifiStatus(void)
{
    AI_UI_WIFI_STATUS_E wifiStatus = AI_UI_WIFI_STATUS_DISCONNECTED;
    
    // Use Arduino WiFi library
    WF_STATION_STAT_E wfStatus = WiFi.status();
    if (wfStatus == WSS_CONN_SUCCESS || wfStatus == WSS_GOT_IP) {
        wifiStatus = AI_UI_WIFI_STATUS_GOOD;
    } else {
        wifiStatus = AI_UI_WIFI_STATUS_DISCONNECTED;
    }
    
    return wifiStatus;
}

/**
 * @brief Update WiFi display based on UI type
 * @param status WiFi status to display
 */
static void updateWifiDisplay(AI_UI_WIFI_STATUS_E status)
{
    appDisplaySetWifi(status);
}

/**
 * @brief Update status display based on UI type
 * @param status Status text to display
 */
static void updateStatusDisplay(const char *status)
{
    if (nullptr == status) {
        PR_ERR("Invalid status text");
        return;
    }
    appDisplaySetStatus(status);
}
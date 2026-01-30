/**
 * @file appStatus.h
 * @brief Status monitoring interface for AI ChatBot
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 */

#ifndef __APP_STATUS_H__
#define __APP_STATUS_H__

#include <stdbool.h>
#include "tuya_cloud_types.h"
#include "TuyaUI.h"
#include "TuyaAI_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize status monitoring module
 * @return OPRT_OK on success, error code on failure
 */
OPERATE_RET appStatusInit();

/**
 * @brief Deinitialize status monitoring module
 */
void appStatusDeinit(void);

/**
 * @brief Check if status monitoring is ready
 * @return true if initialized, false otherwise
 */
bool appStatusIsReady(void);

/**
 * @brief Update WiFi status display
 * @param status WiFi status enum
 */
void appStatusUpdateWifi(UIWifiStatus_t status);

/**
 * @brief Update status text display
 * @param status Status message
 */
void appStatusUpdateText(const char *status);

/**
 * @brief Update chat mode display
 * @param mode Chat mode enum
 */
void appStatusUpdateMode(AIChatMode_t mode);

#ifdef __cplusplus
}
#endif

#endif // __APP_STATUS_H__
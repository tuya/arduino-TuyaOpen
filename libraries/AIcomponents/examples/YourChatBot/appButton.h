/**
 * @file appButton.h
 * @brief Button handling interface for AI ChatBot
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 */

#ifndef __APP_BUTTON_H__
#define __APP_BUTTON_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize button handling
 * @param pin GPIO pin number for the button
 * @return OPRT_OK on success, error code on failure
 */
OPERATE_RET appButtonInit(int pin);

/**
 * @brief Check if button is ready
 * @return true if button initialized, false otherwise
 */
bool appButtonIsReady(void);

/**
 * @brief Get button GPIO pin
 * @return GPIO pin number
 */
int appButtonGetPin(void);

#ifdef __cplusplus
}
#endif

#endif // __APP_BUTTON_H__
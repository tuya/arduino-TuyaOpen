/**
 * @file appDisplay.h
 * @brief Display interface for AI ChatBot
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 */

#ifndef __APP_DISPLAY_H__
#define __APP_DISPLAY_H__

#include "tuya_cloud_types.h"
#include "TuyaAI.h"
#ifdef __cplusplus
extern "C" {
#endif

// UI type selection: BOT_UI_USER(0) for custom, BOT_UI_WECHAT(1) for default WeChat style
#define UI_TYPE             BOT_UI_WECHAT

/**
 * @brief Initialize the display
 * @param uiType UI type (BOT_UI_USER, BOT_UI_WECHAT, etc.)
 * @return OPRT_OK on success, error code on failure
 */
OPERATE_RET appDisplayInit(uint8_t uiType);

/**
 * @brief Check if display is ready
 * @return true if display initialized, false otherwise
 */
bool appDisplayIsReady(void);

/**
 * @brief Clear all display content
 */
void appDisplayClear(void);

/**
 * @brief Display user's spoken text
 * @param text User's text to display
 */
void appDisplaySetUserText(const char *text);

/**
 * @brief Display AI's response text
 * @param text AI's text to display
 */
void appDisplaySetAIText(const char *text);

/**
 * @brief Start streaming AI response
 */
void appDisplayStreamStart(void);

/**
 * @brief Append text to streaming AI response
 * @param text Text chunk to append
 */
void appDisplayStreamAppend(const char *text);

/**
 * @brief End streaming AI response
 */
void appDisplayStreamEnd(void);

/**
 * @brief Set status text
 * @param status Status message to display
 */
void appDisplaySetStatus(const char *status);

/**
 * @brief Set chat mode display
 * @param mode Chat mode to display
 */
void appDisplaySetMode(int mode);

/**
 * @brief Set WiFi status display
 * @param status WiFi status (AI_UI_WIFI_STATUS_GOOD, AI_UI_WIFI_STATUS_FAIR, etc.)
 */
void appDisplaySetWifi(AI_UI_WIFI_STATUS_E status);

#ifdef __cplusplus
}
#endif

#endif // __APP_DISPLAY_H__
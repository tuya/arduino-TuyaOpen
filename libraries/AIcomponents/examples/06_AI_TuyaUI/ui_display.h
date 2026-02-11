/**
 * @file ui_display.h
 * @brief UI display helpers for AI_TuyaUI example
 */

#ifndef _UI_DISPLAY_H_
#define _UI_DISPLAY_H_

#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif

void uiInit(void);
void uiSetUser(const char *text);
void uiSetAI(const char *text);
void uiSetStatus(const char *text);
void uiSetWifi(bool connected);

// AI text buffer management
void uiResetAIBuffer(void);
void uiAppendAIBuffer(const uint8_t *data, uint32_t len);
void uiFlushAIBuffer(void);

#ifdef __cplusplus
}
#endif

#endif

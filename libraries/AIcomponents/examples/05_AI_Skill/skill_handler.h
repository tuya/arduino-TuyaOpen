/**
 * @file skill_handler.h
 * @brief Skill event handlers for AI_Skill example
 */

#ifndef _SKILL_HANDLER_H_
#define _SKILL_HANDLER_H_

#include <Arduino.h>
#include "TuyaAI.h"

#ifdef __cplusplus
extern "C" {
#endif

void skillInit(void);
void skillHandleEvent(void *data);
void skillHandleEmotion(AI_AGENT_EMO_T *emo);
void skillCollectTts(uint8_t *data, uint32_t len);
void skillSaveTts(void);

#ifdef __cplusplus
}
#endif

#endif

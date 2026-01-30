/**
 * @file TuyaAI_Types.h
 * @author Tuya Inc.
 * @brief Common types, enumerations and callbacks for TuyaAI
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 */
#ifndef __TUYA_AI_TYPES_H_
#define __TUYA_AI_TYPES_H_

#include "tuya_cloud_types.h"
#include "utility/include/ai_user_event.h"
/***********************************************************
************************macro define************************
***********************************************************/
#define TUYA_AI_DEFAULT_VOLUME      70
#define TUYA_AI_MAX_VOLUME          100
#define TUYA_AI_MIN_VOLUME          0

/***********************************************************
***********************typedef define***********************
***********************************************************/

/**
 * @brief AI State Enumeration
 * Represents the current state of the AI system
 */
typedef enum {
    AI_STATE_IDLE = 0,          /**< Idle state, waiting for interaction */
    AI_STATE_STANDBY,           /**< Standby state, ready for input */
    AI_STATE_LISTENING,         /**< Listening for user voice input */
    AI_STATE_UPLOADING,         /**< Uploading audio data to cloud */
    AI_STATE_THINKING,          /**< AI is processing/thinking */
    AI_STATE_SPEAKING,          /**< AI is speaking response */
    AI_STATE_MAX
} AIState_t;

/**
 * @brief AI Chat Mode Enumeration
 * @note Values map 1:1 with AI_CHAT_MODE_E from ai_manage_mode.h
 */
typedef enum {
    AI_MODE_HOLD = 0,           /**< Hold button to talk mode */
    AI_MODE_ONESHOT,            /**< Single shot ASR mode */
    AI_MODE_WAKEUP,             /**< Wake word + single talk mode */
    AI_MODE_FREE,               /**< Free conversation mode */
    AI_MODE_MAX
} AIChatMode_t;

/**
 * @brief AI Event Types
 * Events that can be received by the user callback
 */
// 
typedef AI_USER_EVT_TYPE_E AIEvent_t;

/**
 * @brief AI Alert Types for playback
 * @note Values map 1:1 with AI_AUDIO_ALERT_TYPE_E from ai_audio_player.h
 */
typedef enum {
    AI_ALERT_POWER_ON = 0,          /**< Power on notification */
    AI_ALERT_NOT_ACTIVE,            /**< Not activated */
    AI_ALERT_NETWORK_CFG,           /**< Enter network config */
    AI_ALERT_NETWORK_CONNECTED,     /**< Network connected */
    AI_ALERT_NETWORK_FAIL,          /**< Network failed */
    AI_ALERT_NETWORK_DISCONNECT,    /**< Network disconnected */
    AI_ALERT_BATTERY_LOW,           /**< Low battery */
    AI_ALERT_PLEASE_AGAIN,          /**< Please say again */
    AI_ALERT_HOLD_TALK,             /**< Long key press talk */
    AI_ALERT_KEY_TALK,              /**< Key press talk */
    AI_ALERT_WAKEUP_TALK,           /**< Wake-up talk */
    AI_ALERT_RANDOM_TALK,           /**< Random talk */
    AI_ALERT_WAKEUP,                /**< Wakeup greeting */
    AI_ALERT_MAX
} AIAlertType_t;

/***********************************************************
***********************struct define************************
***********************************************************/

/**
 * @brief AI Emotion Data Structure
 */
typedef struct {
    const char *name;           /**< Emotion name (e.g., "HAPPY", "SAD") */
    const char *text;           /**< Emoji or text representation */
} AIEmotion_t;

/**
 * @brief AI Text Data Structure
 */
typedef struct {
    char     *data;             /**< Text data pointer */
    uint16_t dataLen;           /**< Text data length */
    uint32_t timeIndex;         /**< Timestamp index */
} AITextData_t;

/**
 * @brief AI Microphone Data Structure
 */
typedef struct {
    uint8_t  *data;             /**< Audio data pointer */
    uint32_t dataLen;           /**< Audio data length */
} AIMicData_t;

/***********************************************************
***********************callback types***********************
***********************************************************/

/**
 * @brief Event callback function type
 * @param event Event type
 * @param data  Event data pointer (type depends on event)
 * @param len   Event data length
 * @param arg   User argument passed during callback registration
 */
typedef void (*AIEventCallback_t)(AIEvent_t event, uint8_t *data, uint32_t len, void *arg);

/**
 * @brief State change callback function type
 * @param state Current AI state
 */
typedef void (*AIStateCallback_t)(AIState_t state);

/**
 * @brief Custom alert callback function type
 * @param type Alert type to be played
 * @return 0 if custom handling succeeded, other to use default
 */
typedef int (*AIAlertCallback_t)(AIAlertType_t type);

/***********************************************************
***********************config struct************************
***********************************************************/

/**
 * @brief AI Configuration Structure
 */
typedef struct {
    AIChatMode_t        chatMode;       /**< Chat mode selection */
    int                 volume;         /**< Default volume (0-100) */
    AIEventCallback_t   eventCb;        /**< Event callback function */
    AIStateCallback_t   stateCb;        /**< State change callback */
    void               *userArg;        /**< User argument for callbacks */
} AIConfig_t;

#endif /* __TUYA_AI_TYPES_H_ */

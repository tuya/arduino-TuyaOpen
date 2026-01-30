/**
 * @file Button.h
 * @brief Arduino-style Button class for Tuya IoT devices.
 *
 * This file provides an Arduino-friendly wrapper around the Tuya Driver Layer
 * (TDL) button subsystem. It simplifies button configuration and event handling
 * with a clean object-oriented interface.
 *
 * Supported events:
 * - Press down / Press up
 * - Single click / Double click / Multiple clicks
 * - Long press start / Long press hold
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __BUTTON_H__
#define __BUTTON_H__

#include <Arduino.h>

#include "tuya_cloud_types.h"
#include "tal_api.h"
#include "tkl_gpio.h"
#include "tdl_button_manage.h"
#include "tdd_button_gpio.h"

/***********************************************************
 * TYPEDEF
 ***********************************************************/

/**
 * @brief Button event types
 */
typedef enum {
    BUTTON_EVENT_PRESS_DOWN = TDL_BUTTON_PRESS_DOWN,           // Button pressed
    BUTTON_EVENT_PRESS_UP = TDL_BUTTON_PRESS_UP,               // Button released
    BUTTON_EVENT_SINGLE_CLICK = TDL_BUTTON_PRESS_SINGLE_CLICK, // Single click
    BUTTON_EVENT_DOUBLE_CLICK = TDL_BUTTON_PRESS_DOUBLE_CLICK, // Double click
    BUTTON_EVENT_MULTI_CLICK = TDL_BUTTON_PRESS_REPEAT,        // Multiple clicks
    BUTTON_EVENT_LONG_PRESS_START = TDL_BUTTON_LONG_PRESS_START, // Long press started
    BUTTON_EVENT_LONG_PRESS_HOLD = TDL_BUTTON_LONG_PRESS_HOLD,   // Long press holding
    BUTTON_EVENT_MAX = TDL_BUTTON_PRESS_MAX
} ButtonEvent_t;

/**
 * @brief Button active level
 */
typedef struct {
    uint8_t pin;               // GPIO pin number
    TUYA_GPIO_LEVEL_E level;   // Button active level (LOW or HIGH)
    TUYA_GPIO_MODE_E pullMode; // Pull resistor mode (PULLUP or PULLDOWN)
} PinConfig_t;

/**
 * @brief Button configuration structure
 */
typedef struct {
    uint16_t debounceTime;       // Debounce time (ms), default 50
    uint16_t longPressTime;      // Long press trigger time (ms), default 3000
    uint16_t longPressHoldTime;  // Long press hold interval (ms), default 1000
    uint8_t  multiClickCount;    // Multi-click count threshold, default 2
    uint16_t multiClickInterval; // Multi-click valid interval (ms), default 500
} ButtonConfig_t;

/**
 * @brief Convenience structure for simple pin initialization
 * Helper for commonly used initialization patterns
 */
typedef struct {
    uint8_t pin;
    TUYA_GPIO_LEVEL_E activeLevel;
    bool pullUp;  // true for PULLUP, false for PULLDOWN
} SimplePinConfig_t;

/**
 * @brief Button event callback function type
 * @param name Button name
 * @param event Button event type
 * @param arg Event argument (click count for multi-click, hold time for long press)
 */
typedef void (*ButtonEventCallback_t)(char *name, ButtonEvent_t event, void *arg);

/***********************************************************
 * CLASS DECLARATION
 ***********************************************************/

class Button {
public:
    /**
     * @brief Construct a new Button object with default config
     */
    Button();

    /**
     * @brief Destroy the Button object
     */
    ~Button();

    /**
     * @brief Initialize button with detailed configuration
     * @param name Button name (must be unique)
     * @param pinConfig Pin configuration structure
     * @param config Button event configuration structure
     * @return OPRT_OK if successful, error code otherwise
     */
    OPERATE_RET begin(const char *name, PinConfig_t pinConfig, ButtonConfig_t config);

    /**
     * @brief Initialize button with default configuration
     * Uses default timing values for debounce, long press, and multi-click
     * @param name Button name (must be unique)
     * @param pinConfig Pin configuration structure
     * @return OPRT_OK if successful, error code otherwise
     */
    OPERATE_RET begin(const char *name, PinConfig_t pinConfig);

    /**
     * @brief Deinitialize button
     */
    void end();

    /**
     * @brief Register/attach callback for a button event (unified interface)
     * @param event Button event type
     * @param callback Callback function
     * @return true if registration successful
     */
    void setEventCallback(ButtonEvent_t event, ButtonEventCallback_t callback);

    /**
     * @brief Get current button level
     * @return Current button level (TUYA_GPIO_LEVEL_LOW or TUYA_GPIO_LEVEL_HIGH)
     */
    uint8_t getLevel();

private:
    TDL_BUTTON_HANDLE _handle = NULL;


};

#endif // __BUTTON_H__

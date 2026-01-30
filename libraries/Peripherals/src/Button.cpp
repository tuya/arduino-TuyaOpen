/**
 * @file Button.cpp
 * @brief Arduino-style Button class implementation for Tuya IoT devices.
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 *
 */

#include "Button.h"

/***********************************************************
 * Static Members
 ***********************************************************/


/***********************************************************
 * Constructor / Destructor
 ***********************************************************/
Button::Button()
{

}

Button::~Button()
{
    end();
}

/***********************************************************
 * Public Methods
 ***********************************************************/

OPERATE_RET Button::begin(const char *name, PinConfig_t pinConfig, ButtonConfig_t config)
{
    OPERATE_RET rt = OPRT_OK;

    BUTTON_GPIO_CFG_T button_hw_cfg;
    button_hw_cfg.pin = (TUYA_GPIO_NUM_E)pinConfig.pin;
    button_hw_cfg.level = pinConfig.level;
    button_hw_cfg.pin_type.gpio_pull = pinConfig.pullMode;
    button_hw_cfg.mode = BUTTON_TIMER_SCAN_MODE;
    TUYA_CALL_ERR_RETURN(tdd_gpio_button_register((char *)name, &button_hw_cfg));

    TDL_BUTTON_CFG_T button_cfg = {
        .long_start_valid_time = config.longPressTime,
        .long_keep_timer = config.longPressHoldTime,
        .button_debounce_time = config.debounceTime,
        .button_repeat_valid_count = config.multiClickCount,
        .button_repeat_valid_time = config.multiClickInterval
    };
    TUYA_CALL_ERR_RETURN(tdl_button_create((char *)name, &button_cfg, &_handle));
    return OPRT_OK;
}

OPERATE_RET Button::begin(const char *name, PinConfig_t pinConfig)
{
    // Use default button configuration
    ButtonConfig_t defaultConfig = {
        .debounceTime = 50,              // 50ms debounce
        .longPressTime = 3000,           // 3s for long press
        .longPressHoldTime = 1000,       // 1s hold interval
        .multiClickCount = 2,            // Double click threshold
        .multiClickInterval = 500        // 500ms between clicks
    };
    
    return begin(name, pinConfig, defaultConfig);
}

void Button::end()
{
    if (NULL != _handle) {
        tdl_button_delete(_handle);
    }
}

void Button::setEventCallback(ButtonEvent_t event, ButtonEventCallback_t callback)
{
    if (NULL != _handle) {
        tdl_button_event_register(_handle, (TDL_BUTTON_TOUCH_EVENT_E)event, (TDL_BUTTON_EVENT_CB)callback);
    }
}

uint8_t Button::getLevel()
{
    uint8_t level = 0;
    if (NULL != _handle) {
        tdl_button_read_status(_handle, &level);
    }
    return level;
}

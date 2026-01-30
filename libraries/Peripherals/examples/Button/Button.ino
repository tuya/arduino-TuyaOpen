/**
 * @file Button.ino
 * @brief Button example for Tuya IoT devices.
 *
 * This example demonstrates how to use the Button class to handle
 * various button events including single click, double click,
 * long press, and more.
 *
 * Wiring:
 * - Connect a button between the GPIO pin and GND (with internal pull-up)
 * - Or connect between GPIO pin and VCC (with internal pull-down, active high)
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 *
 * @note ===================== Chat bot only support TUYA_T5AI platform =====================
 */

#include "Button.h"
#include "Log.h"

// Button pin configuration (adjust according to your board)
#define BUTTON_PIN0 12
#define BUTTON_PIN1 13

// Create button instance
Button Button0;
Button Button1;

static void buttonCallback(char *name, ButtonEvent_t event, void *arg);

void setup()
{
    // Initialize serial communication
    Serial.begin(115200);
    Log.begin();

    //========== Button configuration method 1 ==========//
    ButtonConfig_t cfg;
    cfg.debounceTime = 50;
    cfg.longPressTime = 2000;
    cfg.longPressHoldTime = 500;
    cfg.multiClickCount = 2;
    cfg.multiClickInterval = 500;

    PinConfig_t pinCfg0;
    pinCfg0.pin = BUTTON_PIN0;
    pinCfg0.level = TUYA_GPIO_LEVEL_LOW;
    pinCfg0.pullMode = TUYA_GPIO_PULLUP;

    // Initialize buttons with name
    Button0.begin("Button0", pinCfg0, cfg);
    // Register button event callbacks
    Button0.setEventCallback(BUTTON_EVENT_PRESS_DOWN, buttonCallback);
    Button0.setEventCallback(BUTTON_EVENT_PRESS_UP, buttonCallback);
    Button0.setEventCallback(BUTTON_EVENT_SINGLE_CLICK, buttonCallback);

    //========== Button configuration method 2: ==========//
    PinConfig_t pinCfg1;
    pinCfg1.pin = BUTTON_PIN1;
    pinCfg1.level = TUYA_GPIO_LEVEL_LOW;
    pinCfg1.pullMode = TUYA_GPIO_PULLUP;

    Button1.begin("Button1", pinCfg1, cfg);
    Button1.setEventCallback(BUTTON_EVENT_PRESS_DOWN, buttonCallback);
    Button1.setEventCallback(BUTTON_EVENT_PRESS_UP, buttonCallback);
    Button1.setEventCallback(BUTTON_EVENT_SINGLE_CLICK, buttonCallback);
}

void loop()
{
    delay(10);
}

static void buttonCallback(char *name, ButtonEvent_t event, void *arg)
{
    PR_DEBUG("[%s] Event: %d", name, event);

    switch (event) {
        case BUTTON_EVENT_PRESS_DOWN:
            PR_DEBUG("Button Down\n");
            break;

        case BUTTON_EVENT_PRESS_UP:
            PR_DEBUG("Button UP\n");
            break;

        case BUTTON_EVENT_SINGLE_CLICK:
            PR_DEBUG("Button Single Click\n");
            break;

        default:
            break;
    }
}

/**
 * @file display_fill.ino
 * @brief Example to fill the display with random colors
 * 
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 * 
 * @note ===================== Only support TUYA_T5AI platform =====================
 */

#include "Display.h"
#include "Log.h"

Display display;

void setup() {
    // Initialize logging
    Serial.begin(115200);
    Log.begin();
    
    PR_NOTICE("========================================");
    PR_NOTICE("Display Fill Example");
    PR_NOTICE("Compile time:        %s %s", __DATE__, __TIME__);
    PR_NOTICE("TuyaOpen version:    %s", OPEN_VERSION);
    PR_NOTICE("Platform chip:       %s", PLATFORM_CHIP);
    PR_NOTICE("Platform board:      %s", PLATFORM_BOARD);
    PR_NOTICE("========================================");

    /*hardware register*/
    if (OPRT_OK != board_register_hardware()) {
        PR_ERR("board_register_hardware error\n");
        while(1);
    }

    // Initialize display
    if (OPRT_OK != display.begin()) {
        PR_ERR("Failed to initialize display");
        return;
    }

    PR_NOTICE("Display resolution: %dx%d", display.getWidth(), display.getHeight());

    // Set brightness to 100%
    display.setBrightness(100);
    
    // Clear screen with black
    display.clear(0x000000);
}

void loop() {
    // Generate random RGB888 color
    uint32_t color = random(0xFFFFFF);
    PR_DEBUG("Filling screen with color: 0x%06X", color);
    
    // Fill screen with random color
    display.fillScreen(color);
    
    delay(2000);
}
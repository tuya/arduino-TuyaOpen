/**
 * @file display_picture.ino
 * @brief Example to display a picture on the screen with rotation
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 * 
 * @note ===================== Only support TUYA_T5AI platform =====================
 */

#include "Display.h"
#include "Log.h"

// Color definitions
#define COLOR_BLACK   0x000000
#define COLOR_WHITE   0xFFFFFF

Display display;

extern const uint16_t imga_width;
extern const uint16_t imga_height;
extern const uint8_t imga_data[];

void setup()
{
    // Initialize logging
    Serial.begin(115200);
    Log.begin();
    
    PR_NOTICE("========================================");
    PR_NOTICE("Display Picture Example");
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

    PR_NOTICE("Display initialized successfully");
    PR_NOTICE("Display resolution: %dx%d", display.getWidth(), display.getHeight());
    PR_NOTICE("Image size: %dx%d", imga_width, imga_height);

    // Set brightness to 100%
    display.setBrightness(100);
    
    // Clear screen with black
    display.clear(COLOR_BLACK);
}

void loop()
{
    static Display::Rotation rotations[] = {
        Display::Rotation::R0,
        Display::Rotation::R90,
        Display::Rotation::R180,
        Display::Rotation::R270
    };
    static const char* rotationNames[] = {"0°", "90°", "180°", "270°"};
    static int rotationIndex = 0;
    
    // Set rotation
    Display::Rotation rotation = rotations[rotationIndex];
    PR_NOTICE("Displaying image with rotation: %s", rotationNames[rotationIndex]);
    
    if (!display.setRotation(rotation)) {
        PR_ERR("Failed to set rotation");
    }
    
    // Clear screen
    display.clear(COLOR_BLACK);
    
    // Draw image (rotation is applied automatically)
    if (!display.drawImage((const uint16_t*)imga_data, imga_width, imga_height, 0, 0)) {
        PR_ERR("Failed to display image");
    } else {
        PR_NOTICE("Image displayed successfully at rotation: %s", rotationNames[rotationIndex]);
    }
    
    // Cycle through rotations
    rotationIndex = (rotationIndex + 1) % 4;
    
    delay(2000);  // Display each rotation for 2 seconds
}
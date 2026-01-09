/**
 * @file display_picture.ino
 * @brief Example to display a picture on the screen
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 * 
 * @note ===================== Only support TUYA_T5AI platform =====================
 */

#include "Display.h"
#include "Log.h"

Display display;

extern const uint16_t imga_width;
extern const uint16_t imga_height;
extern const uint8_t imga_data[];

static void printSystemInfo(void)
{
    PR_NOTICE("========================================");
    PR_NOTICE("Display Picture Example");
    PR_NOTICE("Project name:        %s", PROJECT_NAME);
    PR_NOTICE("App version:         %s", PROJECT_VERSION);
    PR_NOTICE("Compile time:        %s %s", __DATE__, __TIME__);
    PR_NOTICE("TuyaOpen version:    %s", OPEN_VERSION);
    PR_NOTICE("Platform chip:       %s", PLATFORM_CHIP);
    PR_NOTICE("Platform board:      %s", PLATFORM_BOARD);
    PR_NOTICE("========================================");
}

void setup()
{
    OPERATE_RET rt = OPRT_OK;
    
    // Initialize logging
    Serial.begin(115200);
    Log.begin();
    
    printSystemInfo();

    // Initialize display
    rt = display.begin();
    if (rt != OPRT_OK) {
        PR_ERR("Failed to initialize display: %d", rt);
        return;
    }

    PR_NOTICE("Display initialized successfully");
    PR_NOTICE("Display resolution: %dx%d", display.getWidth(), display.getHeight());
    PR_NOTICE("Pixel format: %d", display.getPixelFormat());
    PR_NOTICE("Image size: %dx%d", imga_width, imga_height);

    // Clear screen with black
    display.setBrightness(100);
    display.clear(0x000000);
    display.flush();
}

void loop()
{
    OPERATE_RET rt = OPRT_OK;

    // Check if rotation is needed
    TUYA_DISPLAY_ROTATION_E rotation = display.getRotation();
    
    if (rotation != TUYA_DISPLAY_ROTATION_0) {
        // Draw image with rotation
        PR_DEBUG("Drawing image with rotation: %d", rotation);
        rt = display.drawImageRotated((const uint16_t*)imga_data, imga_width, imga_height, rotation);
    } else {
        // Create image buffer and flush directly
        PR_DEBUG("Drawing image without rotation");
        TDL_DISP_FRAME_BUFF_T *imageFb = display.createImageBuffer((const uint16_t*)imga_data, imga_width, imga_height);
        if (imageFb != NULL) {
            rt = display.flushFrameBuffer(imageFb);
            if (rt != OPRT_OK) {
                PR_ERR("Failed to flush image buffer: %d", rt);
            }
            rt = display.deleteImageBuffer(imageFb);
            if (rt != OPRT_OK) {
                PR_ERR("Failed to delete image buffer: %d", rt);
            }
        } else {
            rt = OPRT_MALLOC_FAILED;
        }
    }

    if (rt != OPRT_OK) {
        PR_ERR("Failed to display image: %d", rt);
    } else {
        PR_DEBUG("Image displayed successfully");
    }

    delay(5000);  // Display image for 5 seconds
}
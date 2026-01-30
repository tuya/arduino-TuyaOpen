/**
 * @file Camera2Display.ino
 * @brief Example: Camera live view to display
 * 
 * This example demonstrates how to use the Camera library to capture
 * YUV422 frames and display them on an LCD screen in real-time.
 * 
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 * 
 */

#include <Arduino.h>
#include "dvpCamera.h"
#include "Display.h"
#include "Log.h"

/***********************************************************
************************macro define************************
***********************************************************/
#define CAMERA_FPS    15

/***********************************************************
***********************variable define**********************
***********************************************************/
Camera camera;
Display display;

void setup()
{
    Serial.begin(115200);
    Log.begin();

    PR_NOTICE("========Camera to Display Example=======");
    PR_NOTICE("Compile time:        %s", __DATE__);
    PR_NOTICE("TuyaOpen version:    %s", OPEN_VERSION);
    PR_NOTICE("Platform chip:       %s", PLATFORM_CHIP);
    PR_NOTICE("Platform board:      %s", PLATFORM_BOARD);
    PR_NOTICE("========================================");

    // Hardware initialization
    if (OPRT_OK != board_register_hardware()) {
        PR_ERR("Board hardware registration failed");
    }
    
    // Initialize display
    if (OPRT_OK != display.begin()) {
        PR_ERR("Failed to initialize display");
        while (1) { delay(1000); }
    }
    display.setBrightness(100);
    display.clear(0x000000);  // Black
    PR_DEBUG("Display initialized");
    
    // Initialize camera
    if (OPRT_OK != camera.begin(CameraResolution::RES_480X480, CAMERA_FPS, CameraFormat::YUV422)) {
        PR_ERR("Failed to initialize camera");
        while (1) { delay(1000); }
    }
    PR_DEBUG("Camera initialized");
}

void loop()
{
    CameraFrame frame;
    
    // Get YUV422 frame from camera
    if (camera.getFrame(frame, CameraFormat::YUV422, 100)) {
        // Display the frame
        if (OPRT_OK != display.displayYUV422Frame(frame.data, frame.width, frame.height)) {
            PR_ERR("Failed to display frame");
        }
    }
}

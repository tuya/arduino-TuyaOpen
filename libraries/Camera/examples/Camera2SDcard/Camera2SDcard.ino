/**
 * @file Camera2SDcard.ino
 * @brief Example: Camera live view with button-controlled capture
 * 
 * This example demonstrates camera live view on display with button-controlled
 * image/video capture to SD card.
 * 
 * Two modes via macro definition:
 * 1. JPEG mode: Display live view + single click button to save JPEG photo
 * 2. H264 mode: Display live view + hold button to record H264 video
 * 
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 */

#include <Arduino.h>
#include "dvpCamera.h"
#include "Display.h"
#include "Button.h"
#include "File.h"
#include "Log.h"

/***********************************************************
************************macro define************************
***********************************************************/
// Uncomment ONE of the following modes:
#define MODE_JPEG_CAPTURE    // Single click to capture JPEG photo
//#define MODE_H264_RECORD     // Hold button to record H264 video

#define CAMERA_FPS    15
#define BUTTON_PIN    12  // GPIO pin for capture button
/***********************************************************
***********************variable define**********************
***********************************************************/
// Global objects
Camera camera;
Display display;
Button captureButton;
VFSFILE sdcard(SDCARD);

// State variables
#ifdef MODE_H264_RECORD
bool isRecording = false;
TUYA_FILE videoFile = NULL;
uint32_t videoFrameCount = 0;
#endif

#ifdef MODE_JPEG_CAPTURE
bool captureRequested = false;
#endif
/***********************************************************
***********************function define**********************
***********************************************************/
void buttonCallback(char *name, ButtonEvent_t event, void *arg);

void setup()
{
    Serial.begin(115200);
    Log.begin();

    // Hardware initialization
    if (OPRT_OK != board_register_hardware()) {
        PR_ERR("Board hardware registration failed");
    }
    
    PR_NOTICE("=======Camera to SD card example========");
    PR_NOTICE("Compile time:        %s", __DATE__);
    PR_NOTICE("TuyaOpen version:    %s", OPEN_VERSION);
    PR_NOTICE("Platform chip:       %s", PLATFORM_CHIP);
    PR_NOTICE("Platform board:      %s", PLATFORM_BOARD);
    PR_NOTICE("========================================");
    
    // Initialize display
    if (OPRT_OK != display.begin()) {
        PR_ERR("Failed to initialize display");
        while (1) { delay(1000); }
    }
    display.setBrightness(100);
    display.clear(0x000000);  // Black
    PR_NOTICE("Display initialized");
    
    // Check SD card (automatically mounted by VFSFILE constructor)
    if (OPRT_OK != sdcard.exist("/")) {
        PR_ERR("SD card not mounted! Please insert SD card.");
        while (1) { delay(1000); }
    }
    PR_NOTICE("SD card ready");
    
    // Initialize camera
#ifdef MODE_JPEG_CAPTURE
    if (OPRT_OK != camera.begin(CameraResolution::RES_480X480, CAMERA_FPS, 
                      CameraFormat::JPEG_YUV422, EncodingQuality::MEDIUM)) {
        PR_ERR("Failed to initialize camera");
        while (1) { delay(1000); }
    }
    PR_NOTICE("Camera initialized: JPEG + YUV422 mode");
#endif

#ifdef MODE_H264_RECORD
    if (!camera.begin(CameraResolution::RES_480X480, CAMERA_FPS, 
                      CameraFormat::H264_YUV422, EncodingQuality::MEDIUM)) {
        PR_ERR("Failed to initialize camera");
        while (1) { delay(1000); }
    }
    PR_NOTICE("Camera initialized: H264 + YUV422 mode");
#endif
    
    PinConfig_t pinCfg;
    pinCfg.pin = BUTTON_PIN;
    pinCfg.level = TUYA_GPIO_LEVEL_LOW;
    pinCfg.pullMode = TUYA_GPIO_PULLUP;
    
    captureButton.begin("CaptureButton", pinCfg);
    
#ifdef MODE_JPEG_CAPTURE
    captureButton.setEventCallback(BUTTON_EVENT_SINGLE_CLICK, buttonCallback);
#endif

#ifdef MODE_H264_RECORD
    captureButton.setEventCallback(BUTTON_EVENT_PRESS_DOWN, buttonCallback);
    captureButton.setEventCallback(BUTTON_EVENT_PRESS_UP, buttonCallback);
#endif
    
    PR_NOTICE("System ready!");
}

void loop()
{
    CameraFrame yuvFrame;
    
    // Always get and display YUV422 frame for live view
    if (camera.getFrame(yuvFrame, CameraFormat::YUV422, 100)) {
        display.displayYUV422Frame(yuvFrame.data, yuvFrame.width, yuvFrame.height);
    }
    
#ifdef MODE_JPEG_CAPTURE
    // Handle JPEG capture
    if (captureRequested) {
        captureRequested = false;
        
        CameraFrame jpegFrame;
        // Get JPEG frame
        if (camera.getFrame(jpegFrame, CameraFormat::JPEG, 500)) {
            // Generate filename with timestamp
            char filename[64];
            snprintf(filename, sizeof(filename), 
                     "/sdcard/photo_%lu.jpg", millis());
            
            // Delete existing file if present
            if (sdcard.exist(filename)) {
                sdcard.remove(filename);
            }
            
            // Open file for writing
            TUYA_FILE file = sdcard.open(filename, "w");
            if (file) {
                int written = sdcard.write((const char*)jpegFrame.data, 
                                          jpegFrame.dataLen, file);
                sdcard.close(file);
                
                if (written == (int)jpegFrame.dataLen) {
                    PR_NOTICE("Photo saved: %s (%d bytes)", 
                             filename, jpegFrame.dataLen);
                } else {
                    PR_ERR("Failed to write complete photo");
                }
            } else {
                PR_ERR("Failed to open file: %s", filename);
            }
        } else {
            PR_WARN("Failed to get JPEG frame");
        }
    }
#endif

#ifdef MODE_H264_RECORD
    // Handle H264 recording
    if (isRecording) {
        CameraFrame h264Frame;
        // Get H264 frame
        if (camera.getFrame(h264Frame, CameraFormat::H264, 100)) {
            if (videoFile) {
                int written = sdcard.write((const char*)h264Frame.data, 
                                          h264Frame.dataLen, videoFile);
                if (written == (int)h264Frame.dataLen) {
                    videoFrameCount++;
                    // Print status every 30 frames (~2 seconds at 15fps)
                    if (videoFrameCount % 30 == 0) {
                        PR_DEBUG("Recording... %d frames", videoFrameCount);
                    }
                } else {
                    PR_ERR("Failed to write H264 frame");
                }
            }
        }
    }
#endif
    
    delay(10);
}

// ==================== BUTTON CALLBACKS ====================
void buttonCallback(char *name, ButtonEvent_t event, void *arg)
{
#ifdef MODE_JPEG_CAPTURE
    // JPEG mode: Single click to capture
    if (event == BUTTON_EVENT_SINGLE_CLICK) {
        captureRequested = true;
        PR_NOTICE("Capture requested!");
    }
#endif

#ifdef MODE_H264_RECORD
    char videoFilename[64];
    // H264 mode: Press down to start recording, release to stop
    if (event == BUTTON_EVENT_PRESS_DOWN) {
        if (!isRecording) {
            // Generate filename with timestamp
            snprintf(videoFilename, sizeof(videoFilename), 
                     "/sdcard/video_%lu.h264", millis());
            
            // Open file for writing
            videoFile = sdcard.open(videoFilename, "w");
            if (videoFile) {
                isRecording = true;
                videoFrameCount = 0;
                PR_NOTICE("Recording started: %s", videoFilename);
            } else {
                PR_ERR("Failed to open file for recording");
            }
        }
    } else if (event == BUTTON_EVENT_PRESS_UP) {
        if (isRecording) {
            sdcard.close(videoFile);
            videoFile = NULL;
            isRecording = false;
            PR_NOTICE("Recording stopped: %s (%d frames)", 
                     videoFilename, videoFrameCount);
        }
    }
#endif
}

/**
 * @file AudioRecorder.ino
 * @brief Example: Button-controlled audio recording with playback
 * 
 * This example demonstrates audio recording controlled by a button.
 * Features:
 * - Press button to start recording
 * - Release button to stop recording and auto-playback
 * - Double-click to stop playback
 * 
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 * @note Only supports TUYA_T5AI platform
 */

#include <Arduino.h>
#include "Audio.h"
#include "Button.h"
#include "Log.h"

/***********************************************************
************************macro define************************
***********************************************************/
#define BUTTON_PIN           12
#define LED_PIN              1

#define EXAMPLE_RECORD_DURATION_MS 3000 // MAX record duration in ms
#define EXAMPLE_MIC_BUFFER_SIZE    EXAMPLE_RECORD_DURATION_MS / 10 * 640 // mic buffer for specified duration

/***********************************************************
***********************variable define**********************
***********************************************************/
Audio audio;
Button recordButton;
static bool needPlayback = false;

/***********************************************************
***********************function define**********************
***********************************************************/
void onButtonEvent(char* name, ButtonEvent_t event, void* arg);

void setup() {
    Serial.begin(115200);
    Log.begin();
    
    // Print startup banner
    PR_NOTICE("======= Audio Recorder Example =========");
    PR_NOTICE("Compile time:        %s", __DATE__);

    // Hardware initialization
    if (OPRT_OK != board_register_hardware()) {
        PR_ERR("Board hardware registration failed");
    }
    
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    
    // Initialize button
    ButtonConfig_t btnCfg = {50, 2000, 500, 2, 300};
    PinConfig_t pinCfg = {BUTTON_PIN, TUYA_GPIO_LEVEL_LOW, TUYA_GPIO_PULLUP};
    
    if (recordButton.begin("Btn", pinCfg, btnCfg) != OPRT_OK) {
        PR_ERR("Failed to initialize button");
        return;
    }
    
    recordButton.setEventCallback(BUTTON_EVENT_PRESS_DOWN, onButtonEvent);
    recordButton.setEventCallback(BUTTON_EVENT_PRESS_UP, onButtonEvent);
    recordButton.setEventCallback(BUTTON_EVENT_DOUBLE_CLICK, onButtonEvent);
    
    // Initialize audio
    AudioConfig cfg;
    cfg.micBufferSize = EXAMPLE_MIC_BUFFER_SIZE;
    cfg.volume = 70;
    
    if (audio.begin(&cfg) != 0) {
        PR_ERR("Failed to initialize audio");
        return;
    }
    
    PR_NOTICE("Press button to record, release to play, double-click to stop");
}

void loop() {
    if (needPlayback && audio.isIdle()) {
        needPlayback = false;
        
        if (audio.getRecordedDataLen() > 0) {
            PR_NOTICE("Start playback...");
            audio.playRecordedData();
            PR_NOTICE("Playback completed");
        }
    }
    
    delay(10);
}

// Button event callback
void onButtonEvent(char* name, ButtonEvent_t event, void* arg) {
    switch (event) {
        case BUTTON_EVENT_PRESS_DOWN:
            if (audio.isIdle()) {
                PR_NOTICE("Start recording...");
                digitalWrite(LED_PIN, HIGH);
                audio.clearRecordedData();
                audio.startRecord();
            }
            break;
            
        case BUTTON_EVENT_PRESS_UP:
            if (audio.isRecording()) {
                PR_NOTICE("Stop recording, recorded %u bytes", audio.getRecordedDataLen());
                digitalWrite(LED_PIN, LOW);
                audio.stopRecord();
                needPlayback = true;
            }
            break;
            
        case BUTTON_EVENT_DOUBLE_CLICK:
            if (audio.isPlaying()) {
                PR_NOTICE("Stop playback");
                audio.stopPlay();
            }
            break;
            
        default:
            break;
    }
}
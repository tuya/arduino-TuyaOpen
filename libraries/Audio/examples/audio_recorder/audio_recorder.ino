/**
 * @file audio_recorder.ino
 * @brief Audio recorder example using C++ Audio class with P12 button control
 * @version 1.0
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 */
#include "Audio.h"
/***********************************************************
**********************Global variables**********************
***********************************************************/
Audio audio;
bool button_pressed_last = false;

// P12 button pin
#define BUTTON_PIN 12

/***********************************************************
**********************Function definitions*****************
***********************************************************/

static void print_system_info(void)
{
    PR_NOTICE("========================================");
    PR_NOTICE("Audio Recorder with P12 Button Control");
    PR_NOTICE("Project name:        %s", PROJECT_NAME);
    PR_NOTICE("App version:         %s", PROJECT_VERSION);
    PR_NOTICE("Compile time:        %s", __DATE__);
    PR_NOTICE("TuyaOpen version:    %s", OPEN_VERSION);
    PR_NOTICE("Platform chip:       %s", PLATFORM_CHIP);
    PR_NOTICE("Platform board:      %s", PLATFORM_BOARD);
    PR_NOTICE("========================================");
}

void setup() {
    OPERATE_RET rt = OPRT_OK;

    // Initialize logging
    tal_log_init(TAL_LOG_LEVEL_DEBUG, 1024, (TAL_LOG_OUTPUT_CB)tkl_log_output);

    print_system_info();

    // Register hardware
    // rt = board_register_hardware();
    // if (rt != OPRT_OK) {
    //     PR_ERR("Failed to register hardware");
    //     return;
    // }

    // Initialize P12 as INPUT with pull-up
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    PR_NOTICE("P12 button initialized as input");

    // Initialize audio with custom config
    Audio::AUDIO_CONFIG_T audio_config = {
        .duration_ms = 5000,  // 5 seconds recording duration
        .volume = 70,         // 70% volume
        .sample_rate = 16000  // 16kHz sample rate
    };

    rt = audio.begin(&audio_config);
    if (rt != OPRT_OK) {
        PR_ERR("Failed to initialize audio");
        return;
    }
}

void loop() {
    // Read P12 button state (LOW = pressed, HIGH = released)
    bool button_pressed = (digitalRead(BUTTON_PIN) == LOW);

    // Handle button press
    if (button_pressed && !button_pressed_last) {
        // Button just pressed
        if (audio.getStatus() == Audio::RECORDER_STATUS_IDLE || 
            audio.getStatus() == Audio::RECORDER_STATUS_PLAYING) {
            PR_NOTICE("Button pressed - Starting recording");
            audio.startRecord();
        }
    }

    // Handle button release
    if (!button_pressed && button_pressed_last) {
        // Button just released
        if (audio.getStatus() == Audio::RECORDER_STATUS_RECORDING) {
            PR_NOTICE("Button released - Stopping recording");
            audio.stopRecord();
        }
    }

    // Update button state for next iteration
    button_pressed_last = button_pressed;

    // Handle state machine
    Audio::RECORDER_STATUS_E status = audio.getStatus();

    switch(status) {
    case Audio::RECORDER_STATUS_START:
        // Recording started
        break;

    case Audio::RECORDER_STATUS_RECORDING: {
        // Print recording progress every second
        uint32_t recorded_len = audio.getRecordedDataLen();
        static uint32_t last_len = 0;
        static uint32_t last_print = 0;
        
        if (millis() - last_print >= 1000) {
            PR_NOTICE("Recording: %u bytes", recorded_len);
            last_print = millis();
            last_len = recorded_len;
        }
        break;
    }

    case Audio::RECORDER_STATUS_END:
        // Recording ended, prepare for playback
        PR_NOTICE("Recording ended");
        PR_NOTICE("Total recorded: %u bytes", audio.getRecordedDataLen());
        PR_NOTICE("Starting playback...");
        audio.setStatus(Audio::RECORDER_STATUS_PLAYING);
        break;

    case Audio::RECORDER_STATUS_PLAYING: {
        // Perform playback
        OPERATE_RET rt = audio.playback();
        if (rt == OPRT_OK) {
            PR_NOTICE("Playback completed");
        } else {
            PR_ERR("Playback failed with error: %d", rt);
        }
        audio.setStatus(Audio::RECORDER_STATUS_IDLE);
        PR_NOTICE("Ready for next recording");
        break;
    }

    case Audio::RECORDER_STATUS_IDLE:
        // Wait for next recording
        break;

    default:
        break;
    }

    delay(50);
}
/**
 * @file audio_recorder.ino
 * @brief Audio recorder example with Button control and frame callback demo
 * 
 * This example demonstrates:
 * 1. Using Button class for audio control
 * 2. Using setAudioFrameCallback for real-time audio processing
 * 3. Using play() interface to play audio data
 * 
 * Button Controls:
 * - Press and hold: Start recording
 * - Release: Stop recording and auto-playback
 * - Double click: Stop playback
 * 
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 * 
 * @note ===================== Only support TUYA_T5AI platform =====================
 */
#include "Audio.h"
#include "Button.h"
#include "Log.h"

/***********************************************************
************************macro define************************
***********************************************************/
#define BUTTON_PIN 12
#define PLAY_FRAME_SIZE 640  // PCM frame size for playback
#define BOARD_SPEAKER_EN_PIN         TUYA_GPIO_NUM_28
/***********************************************************
***********************variable define**********************
***********************************************************/
Audio audio;
Button recordButton;

// Statistics for audio frame callback
static uint32_t total_frames = 0;
static uint32_t total_bytes = 0;
static uint32_t max_audio_level = 0;

// Playback control
static bool need_playback = false;
static uint8_t* play_buffer = NULL;

/***********************************************************
***********************function define**********************
***********************************************************/

/**
 * @brief Audio frame callback - called for each audio frame during recording
 * 
 * This callback is triggered in real-time when audio data is captured.
 * The data is automatically saved to the ring buffer by the Audio class,
 * this callback allows you to process the data additionally.
 * 
 * Use cases:
 * 1. Stream audio to a server (e.g., for cloud speech recognition)
 * 2. Real-time audio level monitoring/visualization
 * 3. Voice Activity Detection (VAD)
 * 4. Real-time audio effects/filtering
 * 
 * @param data Pointer to audio frame data (PCM 16-bit, mono, 16kHz)
 * @param len Length of audio data in bytes
 */
void onAudioFrame(uint8_t* data, uint32_t len) {
  total_frames++;
  total_bytes += len;

  // Calculate audio level (RMS approximation)
  int16_t* samples = (int16_t*)data;
  uint32_t sample_count = len / 2;
  int32_t sum = 0;

  for (uint32_t i = 0; i < sample_count; i++) {
    sum += abs(samples[i]);
  }

  uint32_t avg_level = (sample_count > 0) ? (sum / sample_count) : 0;

  if (avg_level > max_audio_level) {
    max_audio_level = avg_level;
  }

  // Example: Print when significant audio detected
  if (avg_level > 1000) {
    // PR_DEBUG("Frame #%u: level=%u, bytes=%u", total_frames, avg_level, len);
  }
}

/**
 * @brief Reset audio statistics
 */
void resetAudioStats() {
  total_frames = 0;
  total_bytes = 0;
  max_audio_level = 0;
}

/**
 * @brief Print audio statistics
 */
void printAudioStats() {
  PR_NOTICE("===== Audio Statistics =====");
  PR_NOTICE("Total frames: %u", total_frames);
  PR_NOTICE("Total bytes: %u", total_bytes);
  PR_NOTICE("Max audio level: %u", max_audio_level);
  PR_NOTICE("============================");
}

/**
 * @brief Button event callback
 */
void buttonCallback(char* name, ButtonEvent_t event, void* arg) {
  PR_DEBUG("[%s] Event: %d", name, event);

  switch (event) {
    case BUTTON_EVENT_PRESS_DOWN:
      // Start recording when button is pressed
      if (audio.isIdle()) {
        PR_NOTICE("=== Recording Started ===");
        resetAudioStats();
        audio.startRecord();
      }
      break;

    case BUTTON_EVENT_PRESS_UP:
      // Stop recording when button is released
      if (audio.isRecording()) {
        PR_NOTICE("=== Recording Stopped ===");
        audio.stopRecord();
        printAudioStats();
        need_playback = true;  // Flag to start playback in loop
      }
      break;

    case BUTTON_EVENT_DOUBLE_CLICK:
      // Double click to stop playback
      if (audio.isPlaying()) {
        PR_NOTICE("=== Playback Stopped ===");
        audio.stopPlay();
      }
      break;

    default:
      break;
  }
}

void setup() {
  OPERATE_RET rt = OPRT_OK;

  // Initialize logging
  Serial.begin(115200);
  Log.begin(1024);

  PR_NOTICE("========================================");
  PR_NOTICE("Audio Recorder with Frame Callback Demo");
  PR_NOTICE("Project name:        %s", PROJECT_NAME);
  PR_NOTICE("App version:         %s", PROJECT_VERSION);
  PR_NOTICE("Compile time:        %s", __DATE__);
  PR_NOTICE("TuyaOpen version:    %s", OPEN_VERSION);
  PR_NOTICE("Platform chip:       %s", PLATFORM_CHIP);
  PR_NOTICE("Platform board:      %s", PLATFORM_BOARD);
  PR_NOTICE("========================================");

  // Initialize Button using Button class
  ButtonConfig_t btnCfg;
  btnCfg.debounceTime = 50;
  btnCfg.longPressTime = 2000;
  btnCfg.longPressHoldTime = 500;
  btnCfg.multiClickCount = 2;
  btnCfg.multiClickInterval = 300;

  PinConfig_t pinCfg;
  pinCfg.pin = BUTTON_PIN;
  pinCfg.level = TUYA_GPIO_LEVEL_LOW;
  pinCfg.pullMode = TUYA_GPIO_PULLUP;

  rt = recordButton.begin("RecordBtn", pinCfg, btnCfg);
  if (rt != OPRT_OK) {
    PR_ERR("Failed to initialize button: %d", rt);
  }

  // Register button events
  recordButton.setEventCallback(BUTTON_EVENT_PRESS_DOWN, buttonCallback);
  recordButton.setEventCallback(BUTTON_EVENT_PRESS_UP, buttonCallback);
  recordButton.setEventCallback(BUTTON_EVENT_DOUBLE_CLICK, buttonCallback);

  PR_NOTICE("Button initialized on pin %d", BUTTON_PIN);

  // Initialize audio with custom config
  Audio::AUDIO_CONFIG_T audio_config;
  audio_config.duration_ms = 5000;   // 5 seconds max recording
  audio_config.volume = 70;          // 70% volume
  audio_config.sample_rate = 16000;  // 16kHz sample rate
  audio_config.pin = BOARD_SPEAKER_EN_PIN;

  rt = audio.begin(&audio_config);
  if (rt != OPRT_OK) {
    PR_ERR("Failed to initialize audio: %d", rt);
    return;
  }

  // Allocate playback buffer
  play_buffer = (uint8_t*)tkl_system_psram_malloc(PLAY_FRAME_SIZE);
  if (play_buffer == NULL) {
    PR_ERR("Failed to allocate play buffer");
    return;
  }

  // =========================================
  // Set audio frame callback for real-time processing
  // This callback is called for EVERY audio frame during recording
  // =========================================
  audio.setAudioFrameCallback(onAudioFrame);
  PR_NOTICE("Audio initialized successfully");
  PR_NOTICE("Ready for recording...");
}

void loop() {
  // Handle playback after recording
  if (need_playback && audio.getStatus() == Audio::AUDIO_STATUS_RECORD_END) {
    need_playback = false;

    uint32_t data_len = audio.getRecordedDataLen();
    PR_NOTICE("Total recorded: %u bytes", data_len);

    if (data_len > 0 && play_buffer != NULL) {
      PR_NOTICE("Starting playback using play() interface...");
      audio.resetStopFlag();  // Reset stop flag before playback

      uint32_t played_frames = 0;

      // Read and play data frame by frame using play() interface
      while (audio.getRecordedDataLen() > 0 && !audio.shouldStopPlay()) {
        uint32_t read_len = audio.readRecordedData(play_buffer, PLAY_FRAME_SIZE);

        if (read_len > 0) {
          // Use play() interface to play data
          OPERATE_RET rt = audio.play(play_buffer, read_len);
          if (rt != OPRT_OK) {
            PR_ERR("Play failed: %d", rt);
            break;
          }
          played_frames++;
        } else {
          break;
        }
      }

      PR_NOTICE("Playback completed, played %u frames", played_frames);
    } else {
      PR_NOTICE("No data to play or buffer not allocated");
    }

    // Clear and reset to idle
    audio.clearRecordedData();
    audio.stopPlay();  // Ensure status is reset to IDLE
    PR_NOTICE("Ready for next recording...");
  }

  // Print recording progress
  // if (audio.isRecording()) {
  //   static uint32_t last_print = 0;
  //   if (millis() - last_print >= 1000) {
  //     PR_NOTICE("Recording: %u bytes, frames: %u",
  //               audio.getRecordedDataLen(), total_frames);
  //     last_print = millis();
  //   }
  // }

  delay(10);
}
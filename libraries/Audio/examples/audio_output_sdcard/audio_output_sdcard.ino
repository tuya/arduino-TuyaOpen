/**
 * @file audio_output_sdcard.ino
 * @brief Record audio and save to SD card as PCM and WAV files
 * 
 * This example demonstrates:
 * 1. Recording audio with Button control
 * 2. Saving recorded audio to SD card as both PCM and WAV files
 * 3. Auto-incrementing file numbers for each recording
 * 
 * Button Controls:
 * - Press: Start recording
 * - Release: Stop recording and save to SD card
 * 
 * File naming:
 * - recording_001.pcm, recording_001.wav
 * - recording_002.pcm, recording_002.wav
 * - ...
 * 
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 * 
 * @note ===================== Only support TUYA_T5AI platform =====================
 */

#include "Audio.h"
#include "Button.h"
#include "file.h"
#include "Log.h"
#include "wav_encode.h"

/***********************************************************
************************macro define************************
***********************************************************/
#define BUTTON_PIN 12
#define LED_PIN 1
#define BOARD_SPEAKER_EN_PIN         TUYA_GPIO_NUM_28
// Audio configuration
#define AUDIO_SAMPLE_RATE 16000
#define AUDIO_BIT_DEPTH 16
#define AUDIO_CHANNELS 1
#define MAX_RECORD_DURATION_MS 30000  // 30 seconds max

// SD card paths
#define RECORDINGS_DIR "/recordings"

/***********************************************************
***********************variable define**********************
***********************************************************/
Audio audio;
Button recordButton;
VFSFILE fs(SDCARD);

// Recording state (using Audio library status enum)
static Audio::AUDIO_STATUS_E recorder_status = Audio::AUDIO_STATUS_IDLE;
static uint32_t recording_start_time = 0;
static uint32_t total_recorded_bytes = 0;
static uint32_t recording_number = 0;  // Current recording number

// File handles
static TUYA_FILE pcm_file_handle = NULL;

/***********************************************************
***********************function define**********************
***********************************************************/

/**
 * @brief Initialize SD card and create recordings directory
 */
bool initSDCard() {
    Serial.println("Initializing SD card...");
    
    // Check if SD card is mounted
    if (!fs.exist("/")) {
        Serial.println("ERROR: SD card not mounted!");
        Serial.println("Please insert SD card and restart.");
        return false;
    }
    
    Serial.println("SD card mounted successfully");
    
    // Create recordings directory if not exists
    if (!fs.exist(RECORDINGS_DIR)) {
        if (fs.mkdir(RECORDINGS_DIR)) {
            Serial.println("Created recordings directory");
        } else {
            Serial.println("Failed to create recordings directory");
            return false;
        }
    }
    
    return true;
}

/**
 * @brief Open PCM file for recording
 */
bool openPCMFile() {
    char filename[64];
    snprintf(filename, sizeof(filename), "%s/recording_%03d.pcm", RECORDINGS_DIR, recording_number);
    
    Serial.print("Creating PCM file: ");
    Serial.println(filename);
    
    // Remove if exists
    if (fs.exist(filename)) {
        fs.remove(filename);
    }
    
    pcm_file_handle = fs.open(filename, "w");
    if (!pcm_file_handle) {
        Serial.println("Failed to create PCM file");
        return false;
    }
    
    return true;
}

/**
 * @brief Save PCM data from ring buffer to file
 */
void savePCMFromRingBuffer() {
    if (!pcm_file_handle) {
        return;
    }
    
    uint32_t data_len = audio.getRecordedDataLen();
    if (data_len == 0) {
        return;
    }
    
    // Allocate buffer
    uint8_t* buffer = (uint8_t*)tkl_system_psram_malloc(data_len);
    if (!buffer) {
        Serial.println("Failed to allocate buffer");
        return;
    }
    
    // Read from ring buffer and write to file
    uint32_t read_len = audio.readRecordedData(buffer, data_len);
    if (read_len > 0) {
        int written = fs.write((const char*)buffer, read_len, pcm_file_handle);
        if (written != (int)read_len) {
            Serial.println("Write error - disk may be full");
        }
    }
    
    tkl_system_psram_free(buffer);
}

/**
 * @brief Close PCM file
 */
void closePCMFile() {
    if (pcm_file_handle) {
        fs.flush(pcm_file_handle);
        fs.close(pcm_file_handle);
        pcm_file_handle = NULL;
    }
}

/**
 * @brief Create WAV file from PCM file
 */
bool createWAVFromPCM() {
    Serial.println("\n=== Creating WAV file ===");
    
    char pcm_filename[64];
    char wav_filename[64];
    snprintf(pcm_filename, sizeof(pcm_filename), "%s/recording_%03d.pcm", RECORDINGS_DIR, recording_number);
    snprintf(wav_filename, sizeof(wav_filename), "%s/recording_%03d.wav", RECORDINGS_DIR, recording_number);
    
    // Get PCM file size
    int pcm_size = fs.filesize(pcm_filename);
    if (pcm_size <= 0) {
        Serial.println("PCM file is empty or not found");
        return false;
    }
    
    Serial.print("PCM file size: ");
    Serial.print(pcm_size);
    Serial.println(" bytes");
    
    // Open PCM file
    TUYA_FILE pcm_file = fs.open(pcm_filename, "r");
    if (!pcm_file) {
        Serial.println("Failed to open PCM file");
        return false;
    }
    
    // Create WAV file
    TUYA_FILE wav_file = fs.open(wav_filename, "w");
    if (!wav_file) {
        Serial.println("Failed to create WAV file");
        fs.close(pcm_file);
        return false;
    }
    
    // Generate WAV header
    uint8_t wav_header[WAV_HEAD_LEN];
    app_get_wav_head(pcm_size, 1, AUDIO_SAMPLE_RATE, AUDIO_BIT_DEPTH, AUDIO_CHANNELS, wav_header);
    
    // Write WAV header
    int written = fs.write((const char*)wav_header, WAV_HEAD_LEN, wav_file);
    if (written != WAV_HEAD_LEN) {
        Serial.println("Failed to write WAV header");
        fs.close(pcm_file);
        fs.close(wav_file);
        return false;
    }
    
    // Copy PCM data to WAV file
    uint8_t buffer[512];
    int total_copied = 0;
    while (!fs.feof(pcm_file)) {
        int bytes_read = fs.read((const char*)buffer, sizeof(buffer), pcm_file);
        if (bytes_read > 0) {
            int bytes_written = fs.write((const char*)buffer, bytes_read, wav_file);
            if (bytes_written != bytes_read) {
                Serial.println("Write error");
                break;
            }
            total_copied += bytes_written;
        }
    }
    
    fs.close(pcm_file);
    fs.flush(wav_file);
    fs.close(wav_file);
    
    Serial.print("WAV file created, data size: ");
    Serial.print(total_copied);
    Serial.println(" bytes");
    
    return true;
}

/**
 * @brief Audio frame callback for real-time monitoring
 */
void onAudioFrame(uint8_t* data, uint32_t len) {
    total_recorded_bytes += len;
}

/**
 * @brief Button event callback
 */
void buttonCallback(char* name, ButtonEvent_t event, void* arg) {
    Serial.print("Button event: ");
    Serial.println(event);
    
    switch (event) {
        case BUTTON_EVENT_PRESS_DOWN:
            // Start recording
            if (recorder_status == Audio::AUDIO_STATUS_IDLE) {
                Serial.println("\n*** Start Recording ***");
                digitalWrite(LED_PIN, HIGH);
                recorder_status = Audio::AUDIO_STATUS_RECORD_START;
            }
            break;
            
        case BUTTON_EVENT_PRESS_UP:
            // Stop recording
            if (recorder_status == Audio::AUDIO_STATUS_RECORDING) {
                Serial.println("\n*** Stop Recording ***");
                digitalWrite(LED_PIN, LOW);
                recorder_status = Audio::AUDIO_STATUS_RECORD_END;
            }
            break;
            
        default:
            break;
    }
}

void setup() {
    OPERATE_RET rt = OPRT_OK;
    
    // Initialize serial and logging
    Serial.begin(115200);
    Log.begin();
    delay(1000);
    
    PR_NOTICE("========================================");
    PR_NOTICE("Audio Recorder to SD Card");
    PR_NOTICE("Project name:        %s", PROJECT_NAME);
    PR_NOTICE("App version:         %s", PROJECT_VERSION);
    PR_NOTICE("Compile time:        %s", __DATE__);
    PR_NOTICE("TuyaOpen version:    %s", OPEN_VERSION);
    PR_NOTICE("Platform chip:       %s", PLATFORM_CHIP);
    PR_NOTICE("Platform board:      %s", PLATFORM_BOARD);
    PR_NOTICE("========================================");
    
    // Initialize LED
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    
    // Initialize SD card
    if (!initSDCard()) {
        Serial.println("SD card initialization failed!");
        return;
    }
    
    // Initialize Button
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
        Serial.print("Failed to initialize button: ");
        Serial.println(rt);
        return;
    }
    
    // Register button events
    recordButton.setEventCallback(BUTTON_EVENT_PRESS_DOWN, buttonCallback);
    recordButton.setEventCallback(BUTTON_EVENT_PRESS_UP, buttonCallback);
    
    Serial.print("Button initialized on pin ");
    Serial.println(BUTTON_PIN);
    
    // Initialize Audio
    Audio::AUDIO_CONFIG_T audio_config;
    audio_config.duration_ms = MAX_RECORD_DURATION_MS;
    audio_config.volume = 70;
    audio_config.sample_rate = AUDIO_SAMPLE_RATE;
    audio_config.pin = BOARD_SPEAKER_EN_PIN;  // Speaker enable pin
    
    rt = audio.begin(&audio_config);
    if (rt != OPRT_OK) {
        Serial.print("Failed to initialize audio: ");
        Serial.println(rt);
        return;
    }
    
    // Set audio frame callback
    audio.setAudioFrameCallback(onAudioFrame);
}

void loop() {
    switch (recorder_status) {
        case Audio::AUDIO_STATUS_RECORD_START:
            // Open PCM file
            if (openPCMFile()) {
                total_recorded_bytes = 0;
                recording_start_time = millis();
                audio.startRecord();
                recorder_status = Audio::AUDIO_STATUS_RECORDING;
                Serial.println("Recording...");
            } else {
                Serial.println("Failed to start recording");
                recorder_status = Audio::AUDIO_STATUS_IDLE;
                digitalWrite(LED_PIN, LOW);
            }
            break;
            
        case Audio::AUDIO_STATUS_RECORDING:
            // Continuously save data from ring buffer to file
            savePCMFromRingBuffer();
            
            // Monitor recording progress
            {
                static uint32_t last_print = 0;
                uint32_t now = millis();
                
                if (now - last_print >= 1000) {
                    uint32_t duration = now - recording_start_time;
                    Serial.print("Recording: ");
                    Serial.print(duration / 1000);
                    Serial.print("s, ");
                    Serial.print(total_recorded_bytes);
                    Serial.println(" bytes");
                    last_print = now;
                    
                    // Auto-stop if max duration reached
                    if (duration >= MAX_RECORD_DURATION_MS) {
                        Serial.println("Max duration reached!");
                        recorder_status = Audio::AUDIO_STATUS_RECORD_END;
                        digitalWrite(LED_PIN, LOW);
                    }
                }
            }
            break;
            
        case Audio::AUDIO_STATUS_RECORD_END:
            {
                uint32_t duration = millis() - recording_start_time;
                
                // Stop recording
                audio.stopRecord();
                
                // Save remaining data
                savePCMFromRingBuffer();
                
                // Close PCM file
                closePCMFile();
                
                Serial.print("Recording stopped. Duration: ");
                Serial.print(duration);
                Serial.print(" ms, Total: ");
                Serial.print(total_recorded_bytes);
                Serial.println(" bytes");
                
                // Create WAV file from PCM
                if (createWAVFromPCM()) {
                    Serial.println("Files saved successfully");
                    char filename[64];
                    snprintf(filename, sizeof(filename), "recording_%03d.pcm/.wav", recording_number);
                    Serial.print("Saved as: ");
                    Serial.println(filename);
                    
                    // Increment for next recording
                    recording_number++;
                } else {
                    Serial.println("Failed to create WAV file");
                }
                
                // Reset to idle
                audio.clearRecordedData();
                recorder_status = Audio::AUDIO_STATUS_IDLE;
                Serial.println("Ready for next recording\n");
            }
            break;
            
        case Audio::AUDIO_STATUS_IDLE:
        default:
            // Reset ring buffer in idle state
            // audio.clearRecordedData();
            break;
    }
    
    delay(10);
}
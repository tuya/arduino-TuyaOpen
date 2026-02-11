/**
 * @file Audio2SDcard.ino
 * @brief Example: Record audio and save to SD card
 * 
 * This example demonstrates recording audio and saving to SD card.
 * Features:
 * - Press button to start recording
 * - Release button to save as PCM and WAV files
 * - Auto-increment file numbering
 * 
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 * @note Only supports TUYA_T5AI platform
 */

#include <Arduino.h>
#include "Audio.h"
#include "Button.h"
#include "File.h"
#include "Log.h"
#include "wav_encode.h"

/***********************************************************
************************macro define************************
***********************************************************/
#define BUTTON_PIN           12
#define LED_PIN              1
#define RECORDINGS_DIR       "/recordings"
#define MAX_RECORD_MS        30000

/***********************************************************
***********************variable define**********************
***********************************************************/
Audio audio;
Button recordButton;
VFSFILE fs(SDCARD);

enum State { IDLE, RECORDING, SAVING };
static State state = IDLE;
static uint32_t recordStartTime = 0;
static uint32_t fileNumber = 1;
static TUYA_FILE pcmFile = NULL;

/***********************************************************
***********************function define**********************
***********************************************************/
void saveToFile();
bool createWAV();
void onButtonEvent(char* name, ButtonEvent_t event, void* arg);

// Save recorded audio data to file
void saveToFile() {
    uint32_t len = audio.getRecordedDataLen();
    if (len == 0) return;
    
    uint8_t* buf = (uint8_t*)Malloc(len);
    if (buf) {
        uint32_t readLen = audio.readRecordedData(buf, len);
        if (readLen > 0 && pcmFile) {
            fs.write((const char*)buf, readLen, pcmFile);
        }
        Free(buf);
    }
}

// Create WAV file from PCM
bool createWAV() {
    char pcmPath[64], wavPath[64];
    snprintf(pcmPath, sizeof(pcmPath), "%s/rec_%03d.pcm", RECORDINGS_DIR, fileNumber);
    snprintf(wavPath, sizeof(wavPath), "%s/rec_%03d.wav", RECORDINGS_DIR, fileNumber);
    
    int pcmSize = fs.filesize(pcmPath);
    if (pcmSize <= 0) return false;
    
    TUYA_FILE pcm = fs.open(pcmPath, "r");
    TUYA_FILE wav = fs.open(wavPath, "w");
    if (!pcm || !wav) {
        if (pcm) fs.close(pcm);
        if (wav) fs.close(wav);
        return false;
    }
    
    // Write WAV header
    uint8_t header[WAV_HEAD_LEN];
    app_get_wav_head(pcmSize, 1, 16000, 16, 1, header);
    fs.write((const char*)header, WAV_HEAD_LEN, wav);
    
    // Copy PCM data
    uint8_t buf[512];
    while (!fs.feof(pcm)) {
        int n = fs.read((char*)buf, sizeof(buf), pcm);
        if (n > 0) fs.write((const char*)buf, n, wav);
    }
    
    fs.close(pcm);
    fs.close(wav);
    return true;
}

// Button event callback
void onButtonEvent(char* name, ButtonEvent_t event, void* arg) {
    if (event == BUTTON_EVENT_PRESS_DOWN && state == IDLE) {
        // Create PCM file
        char path[64];
        snprintf(path, sizeof(path), "%s/rec_%03d.pcm", RECORDINGS_DIR, fileNumber);
        if (fs.exist(path)) fs.remove(path);
        pcmFile = fs.open(path, "w");
        
        if (pcmFile) {
            PR_NOTICE("Start recording...");
            digitalWrite(LED_PIN, HIGH);
            recordStartTime = millis();
            audio.clearRecordedData();
            audio.startRecord();
            state = RECORDING;
        }
    }
    else if (event == BUTTON_EVENT_PRESS_UP && state == RECORDING) {
        PR_NOTICE("Stop recording");
        digitalWrite(LED_PIN, LOW);
        audio.stopRecord();
        state = SAVING;
    }
}

void setup() {
    Serial.begin(115200);
    Log.begin();
    
    // Print startup banner
    PR_NOTICE("====== Audio to SD Card Example =======");
    PR_NOTICE("Compile time:        %s", __DATE__);

    // Hardware initialization
    if (OPRT_OK != board_register_hardware()) {
        PR_ERR("Board hardware registration failed");
    }
    
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    
    // Check SD card
    if (!fs.exist("/")) {
        PR_ERR("SD card not mounted!");
        return;
    }
    if (!fs.exist(RECORDINGS_DIR)) {
        fs.mkdir(RECORDINGS_DIR);
    }
    
    // Initialize button
    ButtonConfig_t btnCfg = {50, 2000, 500, 2, 300};
    PinConfig_t pinCfg = {BUTTON_PIN, TUYA_GPIO_LEVEL_LOW, TUYA_GPIO_PULLUP};
    recordButton.begin("Btn", pinCfg, btnCfg);
    recordButton.setEventCallback(BUTTON_EVENT_PRESS_DOWN, onButtonEvent);
    recordButton.setEventCallback(BUTTON_EVENT_PRESS_UP, onButtonEvent);
    
    // Initialize audio
    AudioConfig cfg;
    cfg.micBufferSize = 60000;
    cfg.volume = 70;
    
    if (audio.begin(&cfg) != 0) {
        PR_ERR("Failed to initialize audio");
        return;
    }
    
    PR_NOTICE("Press button to record, release to save");
}

void loop() {
    switch (state) {
        case RECORDING:
            saveToFile();
            
            // Auto-stop on timeout
            if (millis() - recordStartTime >= MAX_RECORD_MS) {
                PR_NOTICE("Max recording duration reached");
                digitalWrite(LED_PIN, LOW);
                audio.stopRecord();
                state = SAVING;
            }
            break;
            
        case SAVING:
            saveToFile();  // Save remaining data
            
            if (pcmFile) {
                fs.flush(pcmFile);
                fs.close(pcmFile);
                pcmFile = NULL;
            }
            
            if (createWAV()) {
                PR_NOTICE("Saved: rec_%03d.pcm/wav", fileNumber);
                fileNumber++;
            }
            
            audio.clearRecordedData();
            state = IDLE;
            PR_NOTICE("Ready");
            break;
            
        case IDLE:
        default:
            break;
    }
    
    delay(10);
}

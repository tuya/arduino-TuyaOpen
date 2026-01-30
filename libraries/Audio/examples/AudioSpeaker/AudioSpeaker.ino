/**
 * @file AudioSpeaker.ino
 * @brief Example: MP3 audio playback
 * 
 * This example demonstrates MP3 audio playback from various sources.
 * Features:
 * - Support playback from C array/Flash/SD card
 * - Uses minimp3 decoder
 * 
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 * @note Only supports TUYA_T5AI platform
 */

#include <Arduino.h>
#include "Audio.h"
#include "File.h"
#include "Log.h"

#include "hello_tuya_16k.cpp"
#define MP3_DATA_LEN sizeof(media_src_hello_tuya_16k)

#define MINIMP3_IMPLEMENTATION
#include "minimp3_ex.h"

/***********************************************************
************************macro define************************
***********************************************************/
#define BOARD_SPEAKER_EN_PIN TUYA_GPIO_NUM_28

// Audio source selection: 0=C array, 1=Flash, 2=SD card
#define AUDIO_SOURCE 0

#define MP3_FILE_FLASH  "/hello_tuya.mp3"
#define MP3_FILE_SD     "/music/hello_tuya.mp3"

#define READ_BUF_SIZE   1940
#define PCM_BUF_SIZE    (576 * 2 * 2)

/***********************************************************
***********************variable define**********************
***********************************************************/
Audio audio;
mp3dec_t mp3dec;
uint8_t* readBuf = NULL;
int16_t* pcmBuf = NULL;

/***********************************************************
***********************function define**********************
***********************************************************/
void playMP3(const uint8_t* data, uint32_t dataLen, VFSFILE* vfs = NULL, TUYA_FILE file = NULL);

void setup() {
    Serial.begin(115200);
    Log.begin();
    
    // Print startup banner
    PR_NOTICE("========= Audio Speaker Demo ===========");
    PR_NOTICE("Compile time:        %s", __DATE__);
    PR_NOTICE("TuyaOpen version:    %s", OPEN_VERSION);
    PR_NOTICE("Platform chip:       %s", PLATFORM_CHIP);
    PR_NOTICE("Platform board:      %s", PLATFORM_BOARD);
    PR_NOTICE("========================================");
    
    // Initialize audio
    AudioConfig cfg;
    cfg.micBufferSize = 0;
    cfg.volume = 70;
    cfg.spkPin = BOARD_SPEAKER_EN_PIN;

    // Hardware initialization
    if (OPRT_OK != board_register_hardware()) {
        PR_ERR("Board hardware registration failed");
    }
    
    if (OPRT_OK != audio.begin(&cfg)) {
        PR_ERR("Failed to initialize audio");
        return;
    }
    
    // Allocate buffers
    readBuf = (uint8_t*)Malloc(READ_BUF_SIZE);
    pcmBuf = (int16_t*)Malloc(PCM_BUF_SIZE);
    if (!readBuf || !pcmBuf) {
        PR_ERR("Failed to allocate memory");
        return;
    }
    
    mp3dec_init(&mp3dec);
}

void loop() {
    // Play from selected source
#if AUDIO_SOURCE == 0
    PR_NOTICE("Playing from C array...");
    playMP3(media_src_hello_tuya_16k, MP3_DATA_LEN, NULL, NULL);
    
#elif AUDIO_SOURCE == 1
    PR_NOTICE("Playing from Flash...");
    VFSFILE fsFlash(LITTLEFS);
    if (fsFlash.exist(MP3_FILE_FLASH)) {
        TUYA_FILE f = fsFlash.open(MP3_FILE_FLASH, "r");
        if (f) {
            playMP3(NULL, 0, &fsFlash, f);
            fsFlash.close(f);
        }
    } else {
        PR_ERR("File not exist: %s", MP3_FILE_FLASH);
    }
    
#elif AUDIO_SOURCE == 2
    PR_NOTICE("Playing from SD card...");
    VFSFILE fsSD(SDCARD);
    if (fsSD.exist(MP3_FILE_SD)) {
        TUYA_FILE f = fsSD.open(MP3_FILE_SD, "r");
        if (f) {
            playMP3(NULL, 0, &fsSD, f);
            fsSD.close(f);
        }
    } else {
        PR_ERR("File not exist: %s", MP3_FILE_SD);
    }
#endif
    delay(2000);
}

// Generic MP3 playback function
void playMP3(const uint8_t* data, uint32_t dataLen, VFSFILE* vfs, TUYA_FILE file) {
    uint32_t offset = 0, readSize = 0;
    uint8_t* frameHead = NULL;
    uint32_t remain = 0;
    mp3dec_frame_info_t info;
    
    while (true) {
        // Move remaining data
        if (frameHead && remain > 0) {
            memmove(readBuf, frameHead, remain);
            readSize = remain;
        }
        
        // Read more data
        uint32_t toRead = READ_BUF_SIZE - readSize;
        if (data) {
            // Read from array
            if (offset >= dataLen) {
                if (remain == 0) break;
            } else {
                if (toRead > dataLen - offset) toRead = dataLen - offset;
                memcpy(readBuf + readSize, data + offset, toRead);
                readSize += toRead;
                offset += toRead;
            }
        } else if (vfs && file) {
            // Read from file
            int n = vfs->read((char*)(readBuf + readSize), toRead, file);
            if (n > 0) {
                readSize += n;
            } else if (remain == 0) {
                break;
            }
        }
        
        // Decode
        frameHead = readBuf;
        int samples = mp3dec_decode_frame(&mp3dec, frameHead, readSize, pcmBuf, &info);
        if (samples == 0) break;
        
        frameHead += info.frame_bytes;
        remain = readSize - info.frame_bytes;
        
        // Play
        audio.play((uint8_t*)pcmBuf, samples * 2 * info.channels);
    }
}
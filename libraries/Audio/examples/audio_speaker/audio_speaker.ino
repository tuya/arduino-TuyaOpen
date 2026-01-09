/**
 * @file AudioMP3Player.ino
 * @brief MP3 audio player example supporting multiple audio sources
 * 
 * This example demonstrates MP3 audio playback from three different sources:
 * 1. C array data (embedded in code)
 * 2. Internal flash storage (LittleFS)
 * 3. External SD card (FAT filesystem)
 * 
 * Features:
 * - MP3 decoding using minimp3 library
 * - Automatic audio format detection
 * - Volume control
 * - Multiple playback sources
 * 
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 * 
 * @note ===================== Only support TUYA_T5AI platform =====================
 */

#include "Audio.h"
#include "file.h"
#include "Log.h"

// Include the audio data file directly
#include "hello_tuya_16k.cpp"
#define media_src_hello_tuya_16k_len sizeof(media_src_hello_tuya_16k)

#define MINIMP3_IMPLEMENTATION
#include "minimp3_ex.h"

// Project configuration
#define BOARD_SPEAKER_EN_PIN         TUYA_GPIO_NUM_28

// Audio source selection
#define USE_C_ARRAY        0
#define USE_INTERNAL_FLASH 1
#define USE_SD_CARD        2

// Select audio source here
#define AUDIO_SOURCE USE_C_ARRAY

// File paths
#define MP3_FILE_FLASH  "/hello_tuya.mp3"
#define MP3_FILE_SDCARD "/music/hello_tuya.mp3"

// Audio configuration
#define AUDIO_SAMPLE_RATE  16000
#define AUDIO_VOLUME       70

// Buffer sizes
#define MP3_READ_BUFFER_SIZE 1940
#define PCM_BUFFER_SIZE      (576 * 2 * 2)  // MAX_NSAMP * MAX_NCHAN * MAX_NGRAN

// Global objects
Audio audio;
mp3dec_t mp3_decoder;

// MP3 player state
struct {
    uint8_t* read_buffer;
    uint32_t read_size;
    uint32_t mp3_offset;
    int16_t* pcm_buffer;
    mp3dec_frame_info_t frame_info;
    bool playing;
} player_state;

/***********************************************************
***********************function define**********************
***********************************************************/
void playFromCArray();
void playFromFlash();
void playFromSDCard();

/**
 * @brief Initialize audio system
 */
bool initAudio() {
    Audio::AUDIO_CONFIG_T config = {
        .duration_ms = 5000,
        .volume = AUDIO_VOLUME,
        .sample_rate = AUDIO_SAMPLE_RATE,
        .pin = BOARD_SPEAKER_EN_PIN
    };
    
    OPERATE_RET rt = audio.begin(&config);
    if (rt != OPRT_OK) {
        Serial.print("Failed to initialize audio: ");
        Serial.println(rt);
        return false;
    }
    
    Serial.println("Audio initialized successfully");
    
    return true;
}

/**
 * @brief Initialize MP3 decoder
 */
bool initMP3Decoder() {
    // Allocate buffers
    player_state.read_buffer = (uint8_t*)tkl_system_psram_malloc(MP3_READ_BUFFER_SIZE);
    player_state.pcm_buffer = (int16_t*)tkl_system_psram_malloc(PCM_BUFFER_SIZE);
    
    if (!player_state.read_buffer || !player_state.pcm_buffer) {
        Serial.println("Failed to allocate MP3 buffers!");
        return false;
    }
    
    // Initialize decoder
    mp3dec_init(&mp3_decoder);
    
    player_state.read_size = 0;
    player_state.mp3_offset = 0;
    player_state.playing = false;
    
    Serial.println("MP3 decoder initialized");
    return true;
}

void setup() {
    // Initialize serial
    Serial.begin(115200);
    Log.begin();

    PR_NOTICE("========================================");
    PR_NOTICE("Audio Recorder with Frame Callback Demo");
    PR_NOTICE("Project name:        %s", PROJECT_NAME);
    PR_NOTICE("App version:         %s", PROJECT_VERSION);
    PR_NOTICE("Compile time:        %s", __DATE__);
    PR_NOTICE("TuyaOpen version:    %s", OPEN_VERSION);
    PR_NOTICE("Platform chip:       %s", PLATFORM_CHIP);
    PR_NOTICE("Platform board:      %s", PLATFORM_BOARD);
    PR_NOTICE("========================================");
    
    // Initialize audio system
    if (!initAudio()) {
        Serial.println("Audio initialization failed!");
        return;
    }
    
    // Initialize MP3 decoder
    if (!initMP3Decoder()) {
        Serial.println("MP3 decoder initialization failed!");
        return;
    }
    
    // Play audio based on selected source
#if AUDIO_SOURCE == USE_C_ARRAY
    Serial.println("Source: C Array");
    playFromCArray();
    
#elif AUDIO_SOURCE == USE_INTERNAL_FLASH
    Serial.println("Source: Internal Flash");
    playFromFlash();
    
#elif AUDIO_SOURCE == USE_SD_CARD
    Serial.println("Source: SD Card");
    playFromSDCard();
    
#else
    #error "Invalid AUDIO_SOURCE selection"
#endif
    
    Serial.println("\nPlayback finished!");
}

void loop() {
    delay(1000);
}

/**
 * @brief Play MP3 from C array
 */
void playFromCArray() {
    Serial.println("\n--- Playing from C Array ---");
    
    player_state.mp3_offset = 0;
    player_state.read_size = 0;
    player_state.playing = true;
    
    uint8_t* mp3_frame_head = NULL;
    uint32_t decode_size_remain = 0;
    uint32_t read_size_remain = 0;
    
    do {
        // 1. Move remaining data from last frame to beginning of buffer
        if (mp3_frame_head != NULL && decode_size_remain > 0) {
            memmove(player_state.read_buffer, mp3_frame_head, decode_size_remain);
            player_state.read_size = decode_size_remain;
        }
        
        // 2. Check if we've finished reading the array
        if (player_state.mp3_offset >= media_src_hello_tuya_16k_len) {
            if (decode_size_remain == 0) {
                Serial.println("Playback completed!");
                break;
            } else {
                goto MP3_DECODE;
            }
        }
        
        // 3. Read more MP3 data
        read_size_remain = MP3_READ_BUFFER_SIZE - player_state.read_size;
        if (read_size_remain > media_src_hello_tuya_16k_len - player_state.mp3_offset) {
            read_size_remain = media_src_hello_tuya_16k_len - player_state.mp3_offset;
        }
        
        if (read_size_remain > 0) {
            memcpy(player_state.read_buffer + player_state.read_size,
                   media_src_hello_tuya_16k + player_state.mp3_offset,
                   read_size_remain);
            player_state.read_size += read_size_remain;
            player_state.mp3_offset += read_size_remain;
        }
        
    MP3_DECODE:
        // 4. Decode MP3 frame
        mp3_frame_head = player_state.read_buffer;
        int samples = mp3dec_decode_frame(&mp3_decoder,
                                         mp3_frame_head,
                                         player_state.read_size,
                                         player_state.pcm_buffer,
                                         &player_state.frame_info);
        
        if (samples == 0) {
            Serial.println("MP3 decode failed!");
            break;
        }
        
        // 5. Calculate remaining undecoded data
        mp3_frame_head += player_state.frame_info.frame_bytes;
        decode_size_remain = player_state.read_size - player_state.frame_info.frame_bytes;
        
        // 6. Play PCM data
        uint32_t pcm_size = samples * 2 * player_state.frame_info.channels;
        
        OPERATE_RET rt = audio.play((uint8_t*)player_state.pcm_buffer, pcm_size);
        if (rt != OPRT_OK) {
            Serial.print("Play failed: ");
            Serial.println(rt);
            break;
        }
        
    } while (1);
}

/**
 * @brief Play MP3 from internal flash
 */
void playFromFlash() {
    Serial.println("\n--- Playing from Internal Flash ---");
    
    // Mount LittleFS
    VFSFILE fs(LITTLEFS);
    
    if (!fs.exist(MP3_FILE_FLASH)) {
        Serial.print("File not found: ");
        Serial.println(MP3_FILE_FLASH);
        return;
    }
    
    // Open file
    TUYA_FILE file = fs.open(MP3_FILE_FLASH, "r");
    if (!file) {
        Serial.println("Failed to open file!");
        return;
    }
    
    Serial.print("Playing: ");
    Serial.println(MP3_FILE_FLASH);
    
    player_state.read_size = 0;
    player_state.playing = true;
    
    uint8_t* mp3_frame_head = NULL;
    uint32_t decode_size_remain = 0;
    uint32_t read_size_remain = 0;
    
    do {
        // 1. Move remaining data from last frame to beginning of buffer
        if (mp3_frame_head != NULL && decode_size_remain > 0) {
            memmove(player_state.read_buffer, mp3_frame_head, decode_size_remain);
            player_state.read_size = decode_size_remain;
        }
        
        // 2. Read more data from file
        read_size_remain = MP3_READ_BUFFER_SIZE - player_state.read_size;
        if (read_size_remain > 0) {
            int bytes_read = fs.read((const char*)(player_state.read_buffer + player_state.read_size),
                                    read_size_remain, file);
            if (bytes_read > 0) {
                player_state.read_size += bytes_read;
            } else if (bytes_read == 0) {
                // EOF
                if (decode_size_remain == 0) {
                    Serial.println("Playback completed!");
                    break;
                } else {
                    goto MP3_DECODE_FLASH;
                }
            }
        }
        
    MP3_DECODE_FLASH:
        // 3. Decode MP3 frame
        mp3_frame_head = player_state.read_buffer;
        int samples = mp3dec_decode_frame(&mp3_decoder,
                                         mp3_frame_head,
                                         player_state.read_size,
                                         player_state.pcm_buffer,
                                         &player_state.frame_info);
        
        if (samples == 0) {
            Serial.println("MP3 decode failed!");
            break;
        }
        
        // 4. Calculate remaining undecoded data
        mp3_frame_head += player_state.frame_info.frame_bytes;
        decode_size_remain = player_state.read_size - player_state.frame_info.frame_bytes;
        
        // 5. Play PCM data
        uint32_t pcm_size = samples * 2 * player_state.frame_info.channels;
        OPERATE_RET rt = audio.play((uint8_t*)player_state.pcm_buffer, pcm_size);
        if (rt != OPRT_OK) {
            Serial.print("Play failed: ");
            Serial.println(rt);
            break;
        }
        
    } while (1);
    
    fs.close(file);
}

/**
 * @brief Play MP3 from SD card
 */
void playFromSDCard() {
    Serial.println("\n--- Playing from SD Card ---");
    
    // Mount SD card
    VFSFILE fs(SDCARD);
    
    if (!fs.exist("/")) {
        Serial.println("SD card not mounted!");
        return;
    }
    
    if (!fs.exist(MP3_FILE_SDCARD)) {
        Serial.print("File not found: ");
        Serial.println(MP3_FILE_SDCARD);
        return;
    }
    
    // Open file
    TUYA_FILE file = fs.open(MP3_FILE_SDCARD, "r");
    if (!file) {
        Serial.println("Failed to open file!");
        return;
    }
    
    Serial.print("Playing: ");
    Serial.println(MP3_FILE_SDCARD);
    
    player_state.read_size = 0;
    player_state.playing = true;
    
    uint8_t* mp3_frame_head = NULL;
    uint32_t decode_size_remain = 0;
    uint32_t read_size_remain = 0;
    
    do {
        // 1. Move remaining data from last frame to beginning of buffer
        if (mp3_frame_head != NULL && decode_size_remain > 0) {
            memmove(player_state.read_buffer, mp3_frame_head, decode_size_remain);
            player_state.read_size = decode_size_remain;
        }
        
        // 2. Read more data from SD card
        read_size_remain = MP3_READ_BUFFER_SIZE - player_state.read_size;
        if (read_size_remain > 0) {
            int bytes_read = fs.read((const char*)(player_state.read_buffer + player_state.read_size),
                                    read_size_remain, file);
            if (bytes_read > 0) {
                player_state.read_size += bytes_read;
            } else if (bytes_read == 0) {
                // EOF
                if (decode_size_remain == 0) {
                    Serial.println("Playback completed!");
                    break;
                } else {
                    goto MP3_DECODE_SD;
                }
            }
        }
        
    MP3_DECODE_SD:
        // 3. Decode MP3 frame
        mp3_frame_head = player_state.read_buffer;
        int samples = mp3dec_decode_frame(&mp3_decoder,
                                         mp3_frame_head,
                                         player_state.read_size,
                                         player_state.pcm_buffer,
                                         &player_state.frame_info);
        
        if (samples == 0) {
            Serial.println("MP3 decode failed!");
            break;
        }
        
        // 4. Calculate remaining undecoded data
        mp3_frame_head += player_state.frame_info.frame_bytes;
        decode_size_remain = player_state.read_size - player_state.frame_info.frame_bytes;
        
        // 5. Play PCM data
        uint32_t pcm_size = samples * 2 * player_state.frame_info.channels;
        
        OPERATE_RET rt = audio.play((uint8_t*)player_state.pcm_buffer, pcm_size);
        if (rt != OPRT_OK) {
            Serial.print("Play failed: ");
            Serial.println(rt);
            break;
        }
        
    } while (1);
    
    fs.close(file);
}
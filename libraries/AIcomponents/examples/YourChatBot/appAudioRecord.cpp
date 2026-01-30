/**
 * @file appAudioRecord.cpp
 * @brief Audio recording module implementation
 *
 * Features:
 * - MIC PCM recording with buffer to reduce SD card writes
 * - TTS MP3 recording (direct save)
 * - Thread-safe design with flag-based state management
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 */

#include "appAudioRecord.h"
#include "File.h"
#include "tal_log.h"
#include "tal_memory.h"
#include <string.h>

/***********************************************************
************************macro define************************
***********************************************************/

/***********************************************************
***********************typedef define***********************
***********************************************************/

/***********************************************************
***********************variable define**********************
***********************************************************/
// File system
static VFSFILE sg_fs(SDCARD);
static bool sg_sdCardReady = false;

// File number counters
static uint32_t sg_micFileNumber = 1;
static uint32_t sg_ttsFileNumber = 1;

// MIC recording state
static TUYA_FILE sg_micFile = NULL;
static bool sg_micRecording = false;
static uint32_t sg_micTotalSize = 0;

// MIC buffer for accumulating data before writing
static uint8_t *sg_micBuffer = NULL;
static uint32_t sg_micBufferPos = 0;
static bool sg_micBufferDirty = false;

// TTS recording state
static TUYA_FILE sg_ttsFile = NULL;
static bool sg_ttsRecording = false;
static uint32_t sg_ttsTotalSize = 0;

/***********************************************************
***********************static functions*********************
***********************************************************/

/**
 * @brief Flush MIC buffer to file
 */
static void _flushMicBuffer(void)
{
    if (!sg_micFile || !sg_micBuffer || sg_micBufferPos == 0) {
        return;
    }
    
    int written = sg_fs.write((const char*)sg_micBuffer, sg_micBufferPos, sg_micFile);
    if (written > 0) {
        sg_micTotalSize += written;
    }
    sg_micBufferPos = 0;
    sg_micBufferDirty = false;
}

/***********************************************************
***********************public functions*********************
***********************************************************/

OPERATE_RET appAudioRecordInit(void)
{
    // Check SD card availability
    if (!sg_fs.exist("/")) {
        PR_WARN("SD card not mounted! Audio recording disabled.");
        sg_sdCardReady = false;
        return OPRT_COM_ERROR;
    }
    
    // Create recordings directory if not exists
    if (!sg_fs.exist(AUDIO_RECORD_DIR)) {
        if (sg_fs.mkdir(AUDIO_RECORD_DIR) != 0) {
            PR_ERR("Failed to create recordings directory!");
            sg_sdCardReady = false;
            return OPRT_COM_ERROR;
        }
        PR_NOTICE("Created recordings directory: %s", AUDIO_RECORD_DIR);
    }
    
    // Allocate MIC buffer
    sg_micBuffer = (uint8_t*)Malloc(MIC_BUFFER_SIZE);
    if (!sg_micBuffer) {
        PR_ERR("Failed to allocate MIC buffer!");
        sg_sdCardReady = false;
        return OPRT_MALLOC_FAILED;
    }
    
    sg_sdCardReady = true;
    PR_NOTICE("Audio recording initialized. Dir: %s, Buffer: %d bytes", 
              AUDIO_RECORD_DIR, MIC_BUFFER_SIZE);
    
    return OPRT_OK;
}

bool appAudioRecordAvailable(void)
{
    return sg_sdCardReady;
}

void appMicRecordStart(void)
{
    if (!sg_sdCardReady || sg_micRecording) {
        return;
    }
    
    char path[64];
    snprintf(path, sizeof(path), "%s/mic_%03d.pcm", AUDIO_RECORD_DIR, sg_micFileNumber);
    
    // Remove existing file if any
    if (sg_fs.exist(path)) {
        sg_fs.remove(path);
    }
    
    sg_micFile = sg_fs.open(path, "w");
    if (sg_micFile) {
        sg_micRecording = true;
        sg_micTotalSize = 0;
        sg_micBufferPos = 0;
        sg_micBufferDirty = false;
        PR_DEBUG("MIC recording started: %s", path);
    } else {
        PR_ERR("Failed to create MIC file: %s", path);
    }
}

void appMicRecordStop(void)
{
    if (!sg_micRecording) {
        return;
    }
    
    sg_micRecording = false;
    
    // Flush remaining buffer
    if (sg_micFile && sg_micBuffer && sg_micBufferPos > 0) {
        _flushMicBuffer();
    }
    
    // Close file
    if (sg_micFile) {
        sg_fs.flush(sg_micFile);
        sg_fs.close(sg_micFile);
        sg_micFile = NULL;
    }
    
    if (sg_micTotalSize > 0) {
        PR_NOTICE("MIC saved: mic_%03d.pcm (%d bytes)", sg_micFileNumber, sg_micTotalSize);
        sg_micFileNumber++;
    } else {
        // Delete empty file
        char path[64];
        snprintf(path, sizeof(path), "%s/mic_%03d.pcm", AUDIO_RECORD_DIR, sg_micFileNumber);
        sg_fs.remove(path);
        PR_DEBUG("MIC recording cancelled (no data)");
    }
}

void appMicRecordWrite(const uint8_t *data, uint32_t len)
{
    if (!sg_micRecording || !sg_micFile || !sg_micBuffer || !data || len == 0) {
        return;
    }
    
    // Copy data to buffer instead of direct write
    uint32_t remaining = len;
    const uint8_t *src = data;
    
    while (remaining > 0) {
        uint32_t space = MIC_BUFFER_SIZE - sg_micBufferPos;
        uint32_t copyLen = (remaining < space) ? remaining : space;
        
        memcpy(sg_micBuffer + sg_micBufferPos, src, copyLen);
        sg_micBufferPos += copyLen;
        sg_micBufferDirty = true;
        src += copyLen;
        remaining -= copyLen;
        
        // If buffer is full, mark for flush (will be flushed in main loop)
        if (sg_micBufferPos >= MIC_BUFFER_SIZE) {
            _flushMicBuffer();
        }
    }
}

void appAudioRecordFlush(void)
{
    // Flush MIC buffer if dirty and has data
    if (sg_micRecording && sg_micBufferDirty && sg_micBufferPos > 0) {
        _flushMicBuffer();
    }
}

void appTtsRecordStart(void)
{
    if (!sg_sdCardReady || sg_ttsRecording) {
        return;
    }
    
    char path[64];
    snprintf(path, sizeof(path), "%s/tts_%03d.mp3", AUDIO_RECORD_DIR, sg_ttsFileNumber);
    
    // Remove existing file if any
    if (sg_fs.exist(path)) {
        sg_fs.remove(path);
    }
    
    sg_ttsFile = sg_fs.open(path, "w");
    if (sg_ttsFile) {
        sg_ttsRecording = true;
        sg_ttsTotalSize = 0;
        PR_DEBUG("TTS recording started: %s", path);
    } else {
        PR_ERR("Failed to create TTS file: %s", path);
    }
}

void appTtsRecordStop(void)
{
    if (!sg_ttsRecording) {
        return;
    }
    
    sg_ttsRecording = false;
    
    if (sg_ttsFile) {
        sg_fs.flush(sg_ttsFile);
        sg_fs.close(sg_ttsFile);
        sg_ttsFile = NULL;
    }
    
    if (sg_ttsTotalSize > 0) {
        PR_NOTICE("TTS saved: tts_%03d.mp3 (%d bytes)", sg_ttsFileNumber, sg_ttsTotalSize);
        sg_ttsFileNumber++;
    } else {
        // Delete empty file
        char path[64];
        snprintf(path, sizeof(path), "%s/tts_%03d.mp3", AUDIO_RECORD_DIR, sg_ttsFileNumber);
        sg_fs.remove(path);
        PR_DEBUG("TTS recording cancelled (no data)");
    }
}

void appTtsRecordWrite(const uint8_t *data, uint32_t len)
{
    if (!sg_ttsRecording || !sg_ttsFile || !data || len == 0) {
        return;
    }
    
    int written = sg_fs.write((const char*)data, len, sg_ttsFile);
    if (written > 0) {
        sg_ttsTotalSize += written;
    }
}

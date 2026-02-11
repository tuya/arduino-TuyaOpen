/**
 * @file appAudioRecord.h
 * @brief Audio recording module for saving MIC PCM and TTS MP3 to SD card
 *
 * This module provides functions to record:
 * - MIC audio data as PCM files (16-bit, 16kHz, mono)
 * - TTS audio data as MP3 files (cloud format)
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 */

#ifndef __APP_AUDIO_RECORD_H__
#define __APP_AUDIO_RECORD_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
************************macro define************************
***********************************************************/
// Recording directory on SD card
#define AUDIO_RECORD_DIR    "/ai_recordings"

// Buffer size for MIC data (accumulate before writing to reduce SD card writes)
#define MIC_BUFFER_SIZE     (16000)  // ~500ms of audio at 16kHz 16-bit mono

/***********************************************************
***********************typedef define***********************
***********************************************************/

/***********************************************************
********************function declaration********************
***********************************************************/

/**
 * @brief Initialize audio recording module
 * @return OPRT_OK on success, error code on failure
 */
OPERATE_RET appAudioRecordInit(void);

/**
 * @brief Start MIC PCM recording
 * Creates a new PCM file for recording
 */
void appMicRecordStart(void);

/**
 * @brief Stop MIC PCM recording
 * Flushes buffer and closes file
 */
void appMicRecordStop(void);

/**
 * @brief Write MIC PCM data to buffer/file
 * @param data PCM audio data
 * @param len Data length in bytes
 */
void appMicRecordWrite(const uint8_t *data, uint32_t len);

/**
 * @brief Start TTS MP3 recording
 * Creates a new MP3 file for recording
 */
void appTtsRecordStart(void);

/**
 * @brief Stop TTS MP3 recording
 * Flushes and closes file
 */
void appTtsRecordStop(void);

/**
 * @brief Write TTS audio data to file
 * @param data Audio data (MP3 format from cloud)
 * @param len Data length in bytes
 */
void appTtsRecordWrite(const uint8_t *data, uint32_t len);

/**
 * @brief Check if audio recording is available (SD card mounted)
 * @return true if recording is available
 */
bool appAudioRecordAvailable(void);

/**
 * @brief Flush any pending MIC data to file
 * Call this periodically in main loop to ensure data is written
 */
void appAudioRecordFlush(void);

#ifdef __cplusplus
}
#endif

#endif /* __APP_AUDIO_RECORD_H__ */

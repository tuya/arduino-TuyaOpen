/**
 * @file TuyaVideo.h
 * @author Tuya Inc.
 * @brief TuyaVideo Arduino C++ wrapper for the AI video component
 *
 * @copyright Copyright (c) 2021-2026 Tuya Inc. All Rights Reserved.
 *
 */
#ifndef __TUYA_VIDEO_H_
#define __TUYA_VIDEO_H_

// Upstream headers already self-guard with extern "C", matching TuyaAI.h
#include "tuya_cloud_types.h"
/* ai_video_input.h already pulls in "tdl_camera_manage.h", which is where
 * TDL_CAMERA_FRAME_T (the AI_VIDEO_FLUSH_CB parameter type) is declared. */
#include "ai_video_input.h"

/***********************************************************
***********************class definition*********************
***********************************************************/

/**
 * @class TuyaVideoClass
 * @brief Arduino wrapper around the AI video input component
 *
 * Provides camera-backed JPEG frame capture for AI video use cases.
 */
class TuyaVideoClass
{
public:
    /**
     * @brief Constructor
     */
    TuyaVideoClass();

    /**
     * @brief Destructor
     */
    ~TuyaVideoClass();

    /**
     * @brief Initialize the AI video component
     * @return OPRT_OK on success, error code on failure
     * @note Idempotent - returns OPRT_OK if already initialized
     */
    OPERATE_RET begin();

    /**
     * @brief Deinitialize the AI video component
     * @note Upstream ai_video_input.h exposes no deinit symbol, so this
     *       stops the video pipeline (if running) and clears the local
     *       initialized flag only.
     */
    void end();

    /**
     * @brief Check if the AI video component is initialized
     * @return true if initialized
     */
    bool isInitialized();

    /**
     * @brief Start the video pipeline
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET start();

    /**
     * @brief Stop the video pipeline
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET stop();

    /**
     * @brief Get the latest JPEG frame
     * @param data Output pointer to the JPEG image data buffer
     * @param len  Output pointer to the JPEG image data length
     * @return OPRT_OK on success, error code on failure
     * @note Call freeJpegFrame() on the returned data once done with it
     */
    OPERATE_RET getJpegFrame(uint8_t **data, uint32_t *len);

    /**
     * @brief Free a JPEG frame previously returned by getJpegFrame()
     * @param data Pointer to the JPEG image data buffer to free
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET freeJpegFrame(uint8_t **data);

    /**
     * @brief Register a callback invoked for each raw YUV frame
     * @param cb Callback function (AI_VIDEO_FLUSH_CB from ai_video_input.h)
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET setYuvFrameCallback(AI_VIDEO_FLUSH_CB cb);

private:
    bool _initialized;
    bool _started;
};

#endif /* __TUYA_VIDEO_H_ */

/**
 * @file TuyaPicture.h
 * @brief TuyaPicture C++ wrapper for the on-device photo album (ai_picture)
 * @copyright Copyright (c) 2021-2026 Tuya Inc. All Rights Reserved.
 */

#ifndef __TUYA_PICTURE_H_
#define __TUYA_PICTURE_H_

// Upstream headers already self-guard with extern "C", matching TuyaAI.h
#include "tuya_cloud_types.h"
#include "ai_picture.h"
#include "ai_picture_input.h"
#include "ai_picture_output.h"

/***********************************************************
***********************class definition*********************
***********************************************************/

/**
 * @class TuyaPictureClass
 * @brief Wraps the upstream ai_picture component, which stores JPEG
 *        pictures into an on-device photo album (NOT a cloud upload).
 */
class TuyaPictureClass
{
public:
    TuyaPictureClass();
    ~TuyaPictureClass();

    /**
     * @brief Initialize the picture album module
     * @return OPRT_OK on success (also OPRT_OK if already initialized)
     */
    OPERATE_RET begin();

    /**
     * @brief Deinitialize the picture album module
     * @note Upstream ai_picture has no deinit symbol; this only clears
     *       the local initialized flag.
     */
    void end();

    /**
     * @brief Check if the picture album module is initialized
     * @return true if initialized
     */
    bool isInitialized();

    /**
     * @brief Get the name of the current album
     * @return Album name string
     */
    const char *albumName();

    /**
     * @brief Save a JPEG picture to the on-device album
     * @param data       JPEG data buffer
     * @param len        JPEG data length in bytes
     * @param hint       Desired filename; NULL to auto-generate a timestamp-based name
     * @param outName    Optional buffer to receive the actual filename used
     * @param outNameSize Size of outName buffer
     * @return OPRT_OK on success, OPRT_INVALID_PARM on bad arguments
     */
    OPERATE_RET saveToAlbum(const uint8_t *data, uint32_t len, const char *hint = nullptr, char *outName = nullptr,
                            uint32_t outNameSize = 0);

    /**
     * @brief Send a picture straight to the AI for recognition
     * @param data JPEG data buffer
     * @param len  JPEG data length in bytes
     * @return OPRT_OK on success
     * @note Completion is reported through AI_USER_EVT_SEND_PICTURE_END.
     */
    OPERATE_RET recognize(const uint8_t *data, uint32_t len);

    /**
     * @brief Queue an album picture as an attachment for the next AI request
     * @param filename Album filename, as returned by saveToAlbum()
     * @param text     Question to ask about the picture, may be NULL
     * @return OPRT_OK on success
     * @note Up to AI_PICTURE_INPUT_MAX_NUM pictures can be queued.
     */
    OPERATE_RET attachFromAlbum(const char *filename, const char *text = nullptr);

    /**
     * @brief Remove a previously queued album attachment
     * @param filename Album filename
     * @return OPRT_OK on success
     */
    OPERATE_RET detachFromAlbum(const char *filename);

    /**
     * @brief Send every queued album attachment to the AI
     * @return OPRT_OK on success
     */
    OPERATE_RET sendAttachments();

    /**
     * @brief Set the resolution the AI should generate pictures at
     * @param width  Output width in pixels
     * @param height Output height in pixels
     * @return OPRT_OK on success
     */
    OPERATE_RET setOutputSize(uint16_t width, uint16_t height);

    /**
     * @brief Enable downloading generated pictures from cloud file storage
     * @param width  Output width in pixels
     * @param height Output height in pixels
     * @return OPRT_OK on success
     * @note Requires ENABLE_COMP_AI_PICTURE_HOSTING_DLD in the vendor build.
     */
    OPERATE_RET beginOutputDownload(uint16_t width, uint16_t height);

private:
    bool _initialized;
};

#endif /* __TUYA_PICTURE_H_ */

/**
 * @file dvpCamera.h
 * @brief Arduino Camera class for Tuya Open platform
 * @version 1.0
 * @copyright Copyright (c) 2021-2026 Tuya Inc. All Rights Reserved.
 */

#ifndef __DVP_CAMERA_H__
#define __DVP_CAMERA_H__

#include "tuya_cloud_types.h"
#include "tal_api.h"
#include "board_com_api.h"
/**
 * @brief Camera resolution enumeration
 */
enum class CameraResolution {
    RES_240X240 = 0,    /**< 240x240 resolution */
    RES_480X480 = 1,    /**< 480x480 resolution (default) */
    RES_640X480 = 2,    /**< 640x480 resolution */
    RES_800X480 = 3,    /**< 800x480 resolution */
    RES_864X480 = 4,    /**< 864x480 resolution */
    RES_1280X720 = 5    /**< 1280x720 resolution */
};

/**
 * @brief Camera output format enumeration
 */
enum class CameraFormat {
    YUV422 = 0,        /**< YUV422 raw format */
    JPEG = 1,          /**< JPEG encoded format */
    H264 = 2,          /**< H264 encoded format */
    JPEG_YUV422 = 3,   /**< Both JPEG and YUV422 */
    H264_YUV422 = 4    /**< Both H264 and YUV422 */
};

/**
 * @brief Encoding quality level enumeration
 */
enum class EncodingQuality {
    LOW = 0,     /**< Low quality, smaller file size */
    MEDIUM = 1,  /**< Medium quality (default) */
    HIGH = 2     /**< High quality, larger file size */
};

/**
 * @brief Camera frame structure
 */
struct CameraFrame {
    uint16_t id;            /**< Frame ID */
    bool isIFrame;          /**< Is I-frame (for encoded formats) */
    bool isComplete;        /**< Is frame complete */
    uint16_t width;         /**< Frame width */
    uint16_t height;        /**< Frame height */
    uint32_t dataLen;       /**< Data length in bytes */
    const uint8_t *data;    /**< Pointer to frame data (read-only) */
    
    CameraFrame() : id(0), isIFrame(false), isComplete(false), 
                    width(0), height(0), dataLen(0), data(nullptr) {}
};

/**
 * @brief Camera class for Arduino Tuya Open platform
 */
class Camera {
public:
    /**
     * @brief Constructor
     */
    Camera();

    /**
     * @brief Destructor
     */
    ~Camera();

    /**
     * @brief Initialize camera with default settings (480x480, 15fps, YUV422, Medium quality)
     * @return OPRT_OK on success, other error code on failure
     */
    OPERATE_RET begin();

    /**
     * @brief Initialize camera with specified parameters
     * @param resolution Camera resolution (default: RES_480X480)
     * @param fps Frames per second (default: 15)
     * @param format Output format (default: YUV422)
     * @param quality Encoding quality for JPEG/H264 (default: MEDIUM)
     * @return OPRT_OK on success, other error code on failure
     */
    OPERATE_RET begin(CameraResolution resolution, uint16_t fps = 15, 
               CameraFormat format = CameraFormat::YUV422,
               EncodingQuality quality = EncodingQuality::MEDIUM);

    /**
     * @brief Stop camera and release resources
     */
    void end();

    /**
     * @brief Get camera frame with specified format
     * @param frame Reference to frame structure to store the data
     * @param format Format of frame to retrieve (YUV422, JPEG, or H264)
     * @param timeoutMs Timeout in milliseconds (0 = no wait, default 1000ms)
     * @return OPRT_OK if frame available, other error code otherwise
     * @note If camera is initialized with dual format (JPEG_YUV422/H264_YUV422),
     *       you can get either YUV422 or encoded frame by specifying the format.
     *       If the requested format is not available, returns error with error log.
     */
    OPERATE_RET getFrame(CameraFrame &frame, CameraFormat format, uint32_t timeoutMs = 1000);

    /**
     * @brief Get camera width
     * @return Frame width in pixels
     */
    uint16_t getWidth() const;

    /**
     * @brief Get camera height
     * @return Frame height in pixels
     */
    uint16_t getHeight() const;

    /**
     * @brief Get camera FPS
     * @return Frames per second
     */
    uint16_t getFPS() const;

    /**
     * @brief Get camera output format
     * @return Camera format
     */
    CameraFormat getFormat() const;

    /**
     * @brief Check if camera is initialized
     * @return true if initialized
     */
    bool isInitialized() const;

    // Internal helper methods for callback processing (public for C callbacks)
    void* getImpl() { return _impl; }

private:
    // Internal implementation (opaque pointer)
    void *_impl;
};

#endif // __DVP_CAMERA_H__

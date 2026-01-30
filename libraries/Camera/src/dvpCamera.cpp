/**
 * @file dvpCamera.cpp
 * @brief Arduino Camera class implementation for Tuya Open platform
 * @version 1.0
 * @copyright Copyright (c) 2021-2026 Tuya Inc. All Rights Reserved.
 */

#include "dvpCamera.h"

#if defined (ENABLE_EX_MODULE_CAMERA) && (ENABLE_EX_MODULE_CAMERA ==1)
#include "tdd_camera_gc2145.h"
#endif
#include "tdl_camera_manage.h"
#include "tdl_camera_driver.h"

// Internal implementation structure (hidden from users)
struct CameraImpl {
    TDL_CAMERA_HANDLE_T cameraHandle;
    bool initialized;
    
    uint16_t width;
    uint16_t height;
    uint16_t fps;
    CameraFormat format;
    
    // Frame buffers for caching
    struct FrameBuffer {
        uint8_t *data;
        uint32_t dataLen;
        uint16_t frameId;
        uint16_t width;
        uint16_t height;
        bool isIFrame;
        bool isComplete;
        bool ready;
    };
    
    FrameBuffer rawFrame;
    FrameBuffer encodedFrame;
    
    MUTEX_HANDLE rawMutex;
    MUTEX_HANDLE encodedMutex;
};

// Static instance pointer for callbacks
static Camera *g_cameraInstance = nullptr;

// Internal callback handlers
static OPERATE_RET rawFrameCallback(TDL_CAMERA_HANDLE_T hdl, TDL_CAMERA_FRAME_T *frame)
{
    if (g_cameraInstance == nullptr || g_cameraInstance->getImpl() == nullptr) {
        return OPRT_COM_ERROR;
    }
    
    CameraImpl *impl = static_cast<CameraImpl*>(g_cameraInstance->getImpl());
    
    tal_mutex_lock(impl->rawMutex);
    
    // Free old data if exists
    if (impl->rawFrame.data != nullptr) {
        tal_psram_free(impl->rawFrame.data);
        impl->rawFrame.data = nullptr;
    }
    
    // Allocate and copy new frame data
    impl->rawFrame.dataLen = frame->data_len;
    impl->rawFrame.data = (uint8_t*)tal_psram_malloc(frame->data_len);
    if (impl->rawFrame.data != nullptr) {
        memcpy(impl->rawFrame.data, frame->data, frame->data_len);
        impl->rawFrame.frameId = frame->id;
        impl->rawFrame.width = frame->width;
        impl->rawFrame.height = frame->height;
        impl->rawFrame.isIFrame = frame->is_i_frame;
        impl->rawFrame.isComplete = frame->is_complete;
        impl->rawFrame.ready = true;
    }
    
    tal_mutex_unlock(impl->rawMutex);
    
    return OPRT_OK;
}

static OPERATE_RET encodedFrameCallback(TDL_CAMERA_HANDLE_T hdl, TDL_CAMERA_FRAME_T *frame)
{
    if (g_cameraInstance == nullptr || g_cameraInstance->getImpl() == nullptr) {
        return OPRT_COM_ERROR;
    }
    
    CameraImpl *impl = static_cast<CameraImpl*>(g_cameraInstance->getImpl());
    
    tal_mutex_lock(impl->encodedMutex);
    
    // Free old data if exists
    if (impl->encodedFrame.data != nullptr) {
        tal_psram_free(impl->encodedFrame.data);
        impl->encodedFrame.data = nullptr;
    }
    
    // Allocate and copy new frame data
    impl->encodedFrame.dataLen = frame->data_len;
    impl->encodedFrame.data = (uint8_t*)tal_psram_malloc(frame->data_len);
    if (impl->encodedFrame.data != nullptr) {
        memcpy(impl->encodedFrame.data, frame->data, frame->data_len);
        impl->encodedFrame.frameId = frame->id;
        impl->encodedFrame.width = frame->width;
        impl->encodedFrame.height = frame->height;
        impl->encodedFrame.isIFrame = frame->is_i_frame;
        impl->encodedFrame.isComplete = frame->is_complete;
        impl->encodedFrame.ready = true;
    }
    
    tal_mutex_unlock(impl->encodedMutex);
    
    return OPRT_OK;
}

Camera::Camera()
    : _impl(nullptr)
{
    g_cameraInstance = this;
}

Camera::~Camera()
{
    end();
    if (g_cameraInstance == this) {
        g_cameraInstance = nullptr;
    }
}

OPERATE_RET Camera::begin()
{
    return begin(CameraResolution::RES_480X480, 15, CameraFormat::YUV422, EncodingQuality::MEDIUM);
}

OPERATE_RET Camera::begin(CameraResolution resolution, uint16_t fps, 
                   CameraFormat format, EncodingQuality quality)
{
    if (_impl != nullptr && static_cast<CameraImpl*>(_impl)->initialized) {
        PR_WARN("Camera already initialized");
        return OPRT_OK;
    }

    // Map resolution to width and height
    uint16_t width, height;
    switch (resolution) {
        case CameraResolution::RES_240X240:
            width = 240; height = 240;
            break;
        case CameraResolution::RES_480X480:
            width = 480; height = 480;
            break;
        case CameraResolution::RES_640X480:
            width = 640; height = 480;
            break;
        case CameraResolution::RES_800X480:
            width = 800; height = 480;
            break;
        case CameraResolution::RES_864X480:
            width = 864; height = 480;
            break;
        case CameraResolution::RES_1280X720:
            width = 1280; height = 720;
            break;
        default:
            width = 480; height = 480;
            break;
    }

    // Allocate implementation structure
    _impl = tal_psram_malloc(sizeof(CameraImpl));
    if (_impl == nullptr) {
        PR_ERR("Failed to allocate camera implementation");
        return OPRT_MALLOC_FAILED;
    }
    
    CameraImpl *impl = static_cast<CameraImpl*>(_impl);
    memset(impl, 0, sizeof(CameraImpl));
    
    // Create mutexes
    OPERATE_RET rt = tal_mutex_create_init(&impl->rawMutex);
    if (rt != OPRT_OK) {
        PR_ERR("Failed to create raw mutex: %d", rt);
        tal_psram_free(_impl);
        _impl = nullptr;
        return rt;
    }
    
    rt = tal_mutex_create_init(&impl->encodedMutex);
    if (rt != OPRT_OK) {
        PR_ERR("Failed to create encoded mutex: %d", rt);
        tal_mutex_release(impl->rawMutex);
        tal_psram_free(_impl);
        _impl = nullptr;
        return rt;
    }
    
    // Find camera device
    static char camera_name[] = CAMERA_NAME;
    impl->cameraHandle = tdl_camera_find_dev(camera_name);
    if (impl->cameraHandle == nullptr) {
        PR_ERR("Camera device not found: %s", camera_name);
        tal_mutex_release(impl->rawMutex);
        tal_mutex_release(impl->encodedMutex);
        tal_psram_free(_impl);
        _impl = nullptr;
        return OPRT_NOT_FOUND;
    }
    
    // Get camera capabilities
    TDL_CAMERA_DEV_INFO_T devInfo;
    rt = tdl_camera_dev_get_info(impl->cameraHandle, &devInfo);
    if (rt != OPRT_OK) {
        PR_ERR("Failed to get camera info: %d", rt);
        tal_mutex_release(impl->rawMutex);
        tal_mutex_release(impl->encodedMutex);
        tal_psram_free(_impl);
        _impl = nullptr;
        return rt;
    }
    
    // Validate parameters
    if (width > devInfo.max_width || height > devInfo.max_height || fps > devInfo.max_fps) {
        PR_ERR("Invalid camera parameters: %dx%d@%dfps (max: %dx%d@%dfps)", 
               width, height, fps, devInfo.max_width, devInfo.max_height, devInfo.max_fps);
        tal_mutex_release(impl->rawMutex);
        tal_mutex_release(impl->encodedMutex);
        tal_psram_free(_impl);
        _impl = nullptr;
        return OPRT_INVALID_PARM;
    }
    
    // Configure camera
    TDL_CAMERA_CFG_T config;
    memset(&config, 0, sizeof(TDL_CAMERA_CFG_T));
    config.fps = fps;
    config.width = width;
    config.height = height;
    
    // Configure encoding quality for JPEG/H264
    if (format == CameraFormat::JPEG || format == CameraFormat::H264 ||
        format == CameraFormat::JPEG_YUV422 || format == CameraFormat::H264_YUV422) {
        
        // Configure JPEG quality
        if (format == CameraFormat::JPEG || format == CameraFormat::JPEG_YUV422) {
            config.encoded_quality.jpeg_cfg.enable = 1;
            switch (quality) {
                case EncodingQuality::LOW:
                    config.encoded_quality.jpeg_cfg.max_size = 20;  // 20KB
                    config.encoded_quality.jpeg_cfg.min_size = 10;  // 10KB
                    break;
                case EncodingQuality::MEDIUM:
                    config.encoded_quality.jpeg_cfg.max_size = 50;  // 50KB
                    config.encoded_quality.jpeg_cfg.min_size = 20;  // 20KB
                    break;
                case EncodingQuality::HIGH:
                    config.encoded_quality.jpeg_cfg.max_size = 200; // 200KB
                    config.encoded_quality.jpeg_cfg.min_size = 40;  // 40KB
                    break;
            }
        }
        
        // Configure H264 quality
        if (format == CameraFormat::H264 || format == CameraFormat::H264_YUV422) {
            config.encoded_quality.h264_cfg.enable = 1;
            switch (quality) {
                case EncodingQuality::LOW:
                    config.encoded_quality.h264_cfg.init_qp = 35;
                    config.encoded_quality.h264_cfg.i_min_qp = 20;
                    config.encoded_quality.h264_cfg.i_max_qp = 45;
                    config.encoded_quality.h264_cfg.p_min_qp = 20;
                    config.encoded_quality.h264_cfg.p_max_qp = 45;
                    config.encoded_quality.h264_cfg.i_block_bits = 100;
                    config.encoded_quality.h264_cfg.p_block_bits = 80;
                    break;
                case EncodingQuality::MEDIUM:
                    config.encoded_quality.h264_cfg.init_qp = 28;
                    config.encoded_quality.h264_cfg.i_min_qp = 15;
                    config.encoded_quality.h264_cfg.i_max_qp = 38;
                    config.encoded_quality.h264_cfg.p_min_qp = 15;
                    config.encoded_quality.h264_cfg.p_max_qp = 38;
                    config.encoded_quality.h264_cfg.i_block_bits = 150;
                    config.encoded_quality.h264_cfg.p_block_bits = 120;
                    break;
                case EncodingQuality::HIGH:
                    config.encoded_quality.h264_cfg.init_qp = 22;
                    config.encoded_quality.h264_cfg.i_min_qp = 10;
                    config.encoded_quality.h264_cfg.i_max_qp = 32;
                    config.encoded_quality.h264_cfg.p_min_qp = 10;
                    config.encoded_quality.h264_cfg.p_max_qp = 32;
                    config.encoded_quality.h264_cfg.i_block_bits = 200;
                    config.encoded_quality.h264_cfg.p_block_bits = 160;
                    break;
            }
        }
    }
    
    // Map CameraFormat to TDL format
    switch (format) {
        case CameraFormat::YUV422:
            config.out_fmt = TDL_CAMERA_FMT_YUV422;
            config.get_frame_cb = rawFrameCallback;
            config.get_encoded_frame_cb = nullptr;
            break;
        case CameraFormat::JPEG:
            config.out_fmt = TDL_CAMERA_FMT_JPEG;
            config.get_frame_cb = nullptr;
            config.get_encoded_frame_cb = encodedFrameCallback;
            break;
        case CameraFormat::H264:
            config.out_fmt = TDL_CAMERA_FMT_H264;
            config.get_frame_cb = nullptr;
            config.get_encoded_frame_cb = encodedFrameCallback;
            break;
        case CameraFormat::JPEG_YUV422:
            config.out_fmt = TDL_CAMERA_FMT_JPEG_YUV422_BOTH;
            config.get_frame_cb = rawFrameCallback;
            config.get_encoded_frame_cb = encodedFrameCallback;
            break;
        case CameraFormat::H264_YUV422:
            config.out_fmt = TDL_CAMERA_FMT_H264_YUV422_BOTH;
            config.get_frame_cb = rawFrameCallback;
            config.get_encoded_frame_cb = encodedFrameCallback;
            break;
        default:
            config.out_fmt = TDL_CAMERA_FMT_YUV422;
            config.get_frame_cb = rawFrameCallback;
            config.get_encoded_frame_cb = nullptr;
            break;
    }
    
    // Open camera
    rt = tdl_camera_dev_open(impl->cameraHandle, &config);
    if (rt != OPRT_OK) {
        PR_ERR("Failed to open camera: %d", rt);
        tal_mutex_release(impl->rawMutex);
        tal_mutex_release(impl->encodedMutex);
        tal_psram_free(_impl);
        _impl = nullptr;
        return rt;
    }
    
    // Store configuration
    impl->width = width;
    impl->height = height;
    impl->fps = fps;
    impl->format = format;
    impl->initialized = true;
    
    // Set global instance for callbacks
    g_cameraInstance = this;
    
    PR_NOTICE("Camera initialized: %dx%d @ %d fps, format: %d", width, height, fps, (int)format);
    return OPRT_OK;
}

void Camera::end()
{
    if (_impl == nullptr) {
        return;
    }
    
    CameraImpl *impl = static_cast<CameraImpl*>(_impl);
    
    if (impl->initialized) {
        tdl_camera_dev_close(impl->cameraHandle);
        impl->initialized = false;
    }
    
    // Free frame buffers
    if (impl->rawFrame.data != nullptr) {
        tal_psram_free(impl->rawFrame.data);
    }
    if (impl->encodedFrame.data != nullptr) {
        tal_psram_free(impl->encodedFrame.data);
    }
    
    // Release mutexes
    if (impl->rawMutex != nullptr) {
        tal_mutex_release(impl->rawMutex);
    }
    if (impl->encodedMutex != nullptr) {
        tal_mutex_release(impl->encodedMutex);
    }
    
    tal_psram_free(_impl);
    _impl = nullptr;
    
    PR_DEBUG("Camera closed");
}

OPERATE_RET Camera::getFrame(CameraFrame &frame, CameraFormat format, uint32_t timeoutMs)
{
    if (_impl == nullptr) {
        PR_ERR("Camera not initialized");
        return OPRT_COM_ERROR;
    }
    
    CameraImpl *impl = static_cast<CameraImpl*>(_impl);
    
    if (!impl->initialized) {
        PR_ERR("Camera not initialized");
        return OPRT_COM_ERROR;
    }
    
    // Check if requested format is available
    bool canGetYUV422 = false;
    bool canGetEncoded = false;
    
    switch (impl->format) {
        case CameraFormat::YUV422:
            canGetYUV422 = true;
            break;
        case CameraFormat::JPEG:
        case CameraFormat::H264:
            canGetEncoded = true;
            break;
        case CameraFormat::JPEG_YUV422:
        case CameraFormat::H264_YUV422:
            canGetYUV422 = true;
            canGetEncoded = true;
            break;
    }
    
    // Determine which buffer to use based on requested format
    CameraImpl::FrameBuffer *targetBuffer = nullptr;
    MUTEX_HANDLE targetMutex = nullptr;
    
    if (format == CameraFormat::YUV422) {
        if (!canGetYUV422) {
            PR_ERR("YUV422 format not available. Camera initialized with format: %d", (int)impl->format);
            return OPRT_NOT_SUPPORTED;
        }
        targetBuffer = &impl->rawFrame;
        targetMutex = impl->rawMutex;
    } else if (format == CameraFormat::JPEG || format == CameraFormat::H264) {
        if (!canGetEncoded) {
            PR_ERR("Encoded format not available. Camera initialized with format: %d", (int)impl->format);
            return OPRT_NOT_SUPPORTED;
        }
        targetBuffer = &impl->encodedFrame;
        targetMutex = impl->encodedMutex;
    } else {
        PR_ERR("Invalid format requested: %d", (int)format);
        return OPRT_INVALID_PARM;
    }
    
    // Wait for frame with timeout
    uint32_t startTime = tal_system_get_millisecond();
    while (!targetBuffer->ready) {
        if (timeoutMs > 0 && (tal_system_get_millisecond() - startTime) >= timeoutMs) {
            return OPRT_TIMEOUT;
        }
        tal_system_sleep(10);
    }
    
    tal_mutex_lock(targetMutex);
    
    if (targetBuffer->ready && targetBuffer->data != nullptr) {
        frame.id = targetBuffer->frameId;
        frame.isIFrame = targetBuffer->isIFrame;
        frame.isComplete = targetBuffer->isComplete;
        frame.width = targetBuffer->width;
        frame.height = targetBuffer->height;
        frame.dataLen = targetBuffer->dataLen;
        frame.data = targetBuffer->data;
        
        targetBuffer->ready = false;  // Mark as consumed
        
        tal_mutex_unlock(targetMutex);
        return OPRT_OK;
    }
    
    tal_mutex_unlock(targetMutex);
    return OPRT_COM_ERROR;
}

uint16_t Camera::getWidth() const
{
    if (_impl == nullptr) {
        return 0;
    }
    return static_cast<CameraImpl*>(_impl)->width;
}

uint16_t Camera::getHeight() const
{
    if (_impl == nullptr) {
        return 0;
    }
    return static_cast<CameraImpl*>(_impl)->height;
}

uint16_t Camera::getFPS() const
{
    if (_impl == nullptr) {
        return 0;
    }
    return static_cast<CameraImpl*>(_impl)->fps;
}

CameraFormat Camera::getFormat() const
{
    if (_impl == nullptr) {
        return CameraFormat::YUV422;
    }
    return static_cast<CameraImpl*>(_impl)->format;
}

bool Camera::isInitialized() const
{
    if (_impl == nullptr) {
        return false;
    }
    return static_cast<CameraImpl*>(_impl)->initialized;
}

/**
 * @file Display.cpp
 * @brief Arduino Display class implementation for Tuya Open platform
 * @version 1.0
 * @copyright Copyright (c) 2021-2026 Tuya Inc. All Rights Reserved.
 */
#include "Display.h"

#include "tdl_display_manage.h"
#include "tdl_display_draw.h"
#include "tdl_display_driver.h"
#include "tdl_display_format.h"
#include "tdl_display_fb_manage.h"
#include "tuya_t5ai_ex_module.h"

// Internal implementation structure (hidden from users)
struct DisplayImpl {
    TDL_DISP_HANDLE_T dispHandle;
    TDL_DISP_FRAME_BUFF_T *frameBuffer;
    TDL_DISP_DEV_INFO_T devInfo;
    TDL_FB_MANAGE_HANDLE_T fbManage;
    
    uint16_t width;
    uint16_t height;
    TUYA_DISPLAY_PIXEL_FMT_E pixelFormat;
    bool isSwap;
    bool initialized;
    uint32_t frameLength;
    uint8_t bitsPerPixel;
    uint8_t bytesPerPixel;
    uint8_t pixelsPerByte;
    uint8_t numFrameBuffers;
};

static char display_name[] = DISPLAY_NAME;

Display::Display() 
    : _impl(nullptr)
{

}

Display::~Display()
{
    end();
}

OPERATE_RET Display::begin()
{
    if (_impl != nullptr && static_cast<DisplayImpl*>(_impl)->initialized) {
        PR_WARN("Display already initialized");
        return OPRT_OK;
    }

#if !defined(DISPLAY_NAME)
    PR_ERR("DISPLAY_NAME not defined");
    return OPRT_NOT_SUPPORTED;
#endif

    // Allocate implementation structure
    _impl = tal_psram_malloc(sizeof(DisplayImpl));
    if (_impl == nullptr) {
        PR_ERR("Failed to allocate display implementation");
        return OPRT_MALLOC_FAILED;
    }
    
    DisplayImpl *impl = static_cast<DisplayImpl*>(_impl);
    memset(impl, 0, sizeof(DisplayImpl));

    OPERATE_RET rt = OPRT_OK;
    
    // Find display device
    impl->dispHandle = tdl_disp_find_dev(display_name);
    if (impl->dispHandle == nullptr) {
        PR_ERR("Display device %s not found", DISPLAY_NAME);
        tal_psram_free(_impl);
        _impl = nullptr;
        return OPRT_NOT_FOUND;
    }

    // Get display device information
    rt = tdl_disp_dev_get_info(impl->dispHandle, &impl->devInfo);
    if (rt != OPRT_OK) {
        PR_ERR("Failed to get display info: %d", rt);
        tal_psram_free(_impl);
        _impl = nullptr;
        return rt;
    }

    // Store display properties
    impl->width = impl->devInfo.width;
    impl->height = impl->devInfo.height;
    impl->pixelFormat = impl->devInfo.fmt;
    impl->isSwap = impl->devInfo.is_swap;

    // Open display device
    rt = tdl_disp_dev_open(impl->dispHandle);
    if (rt != OPRT_OK) {
        PR_ERR("Failed to open display device: %d", rt);
        tal_psram_free(_impl);
        _impl = nullptr;
        return rt;
    }

    // Set default brightness to 100%
    tdl_disp_set_brightness(impl->dispHandle, 100);

    // Calculate frame buffer size
    impl->bitsPerPixel = tdl_disp_get_fmt_bpp(impl->pixelFormat);
    if (impl->bitsPerPixel == 0) {
        PR_ERR("Unsupported pixel format: %d", impl->pixelFormat);
        tdl_disp_dev_close(impl->dispHandle);
        tal_psram_free(_impl);
        _impl = nullptr;
        return OPRT_NOT_SUPPORTED;
    }

    if (impl->bitsPerPixel < 8) {
        impl->pixelsPerByte = 8 / impl->bitsPerPixel;
        impl->frameLength = (impl->width + impl->pixelsPerByte - 1) / impl->pixelsPerByte * impl->height;
    } else {
        impl->bytesPerPixel = (impl->bitsPerPixel + 7) / 8;
        impl->frameLength = impl->width * impl->height * impl->bytesPerPixel;
    }

    // Create frame buffer
    impl->frameBuffer = tdl_disp_create_frame_buff(DISP_FB_TP_PSRAM, impl->frameLength);
    if (impl->frameBuffer == nullptr) {
        PR_ERR("Failed to create frame buffer");
        tdl_disp_dev_close(impl->dispHandle);
        tal_psram_free(_impl);
        _impl = nullptr;
        return OPRT_MALLOC_FAILED;
    }

    // Initialize frame buffer properties
    impl->frameBuffer->x_start = 0;
    impl->frameBuffer->y_start = 0;
    impl->frameBuffer->fmt = impl->pixelFormat;
    impl->frameBuffer->width = impl->width;
    impl->frameBuffer->height = impl->height;

    // Initialize frame buffer management for camera frames
    rt = tdl_disp_fb_manage_init(&impl->fbManage);
    if (rt != OPRT_OK) {
        PR_WARN("Frame buffer manager init failed: %d, camera display may not work", rt);
        impl->fbManage = nullptr;
    } else {
        // Create additional frame buffers for camera display
        impl->numFrameBuffers = (impl->devInfo.rotation != TUYA_DISPLAY_ROTATION_0) ? 3 : 2;
        for (uint8_t i = 0; i < impl->numFrameBuffers; i++) {
            OPERATE_RET fbRt = tdl_disp_fb_manage_add(impl->fbManage, impl->pixelFormat, impl->width, impl->height);
            if (fbRt != OPRT_OK) {
                PR_WARN("Failed to add frame buffer %d: %d", i, fbRt);
            }
        }
        PR_DEBUG("Frame buffer manager initialized with %d buffers", impl->numFrameBuffers);
    }

    impl->initialized = true;
    PR_DEBUG("Display initialized: %dx%d, fmt=%d, bpp=%d", impl->width, impl->height, impl->pixelFormat, impl->bitsPerPixel);
    
    return OPRT_OK;
}

void Display::end()
{
    if (_impl == nullptr) {
        return;
    }

    DisplayImpl *impl = static_cast<DisplayImpl*>(_impl);
    
    if (!impl->initialized) {
        tal_psram_free(_impl);
        _impl = nullptr;
        return;
    }

    if (impl->fbManage != nullptr) {
        // Frame buffers managed by fb manager will be freed automatically
        impl->fbManage = nullptr;
    }

    if (impl->frameBuffer != nullptr) {
        tdl_disp_free_frame_buff(impl->frameBuffer);
        impl->frameBuffer = nullptr;
    }

    if (impl->dispHandle != nullptr) {
        tdl_disp_dev_close(impl->dispHandle);
        impl->dispHandle = nullptr;
    }

    impl->initialized = false;
    PR_DEBUG("Display closed");
    
    tal_psram_free(_impl);
    _impl = nullptr;
}

OPERATE_RET Display::setBrightness(uint8_t brightness)
{
    if (_impl == nullptr) {
        PR_ERR("Display not initialized");
        return OPRT_COM_ERROR;
    }

    DisplayImpl *impl = static_cast<DisplayImpl*>(_impl);
    if (!impl->initialized) {
        PR_ERR("Display not initialized");
        return OPRT_COM_ERROR;
    }

    return tdl_disp_set_brightness(impl->dispHandle, brightness);
}

OPERATE_RET Display::clear(uint32_t color)
{
    if (_impl == nullptr) {
        PR_ERR("Display not initialized");
        return OPRT_COM_ERROR;
    }

    DisplayImpl *impl = static_cast<DisplayImpl*>(_impl);
    if (!impl->initialized || impl->frameBuffer == nullptr) {
        PR_ERR("Display not initialized");
        return OPRT_COM_ERROR;
    }

    // Convert RGB888 to display format
    uint32_t dispColor = tdl_disp_convert_color_fmt(color, TUYA_PIXEL_FMT_RGB888, 
                                                     impl->pixelFormat, 32768);
    
    OPERATE_RET rt = tdl_disp_draw_fill_full(impl->frameBuffer, dispColor, impl->isSwap);
    if (rt != OPRT_OK) {
        return rt;
    }
    
    return tdl_disp_dev_flush(impl->dispHandle, impl->frameBuffer);
}

OPERATE_RET Display::drawPixel(uint16_t x, uint16_t y, uint32_t color)
{
    if (_impl == nullptr) {
        PR_ERR("Display not initialized");
        return OPRT_COM_ERROR;
    }

    DisplayImpl *impl = static_cast<DisplayImpl*>(_impl);
    if (!impl->initialized || impl->frameBuffer == nullptr) {
        PR_ERR("Display not initialized");
        return OPRT_COM_ERROR;
    }

    if (x >= impl->width || y >= impl->height) {
        PR_ERR("Pixel coordinates out of bounds: (%d, %d)", x, y);
        return OPRT_INVALID_PARM;
    }

    // Convert RGB888 to display format
    uint32_t dispColor = tdl_disp_convert_color_fmt(color, TUYA_PIXEL_FMT_RGB888, 
                                                     impl->pixelFormat, 32768);

    return tdl_disp_draw_point(impl->frameBuffer, x, y, dispColor, impl->isSwap);
}

OPERATE_RET Display::fillRect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint32_t color)
{
    if (_impl == nullptr) {
        PR_ERR("Display not initialized");
        return OPRT_COM_ERROR;
    }

    DisplayImpl *impl = static_cast<DisplayImpl*>(_impl);
    if (!impl->initialized || impl->frameBuffer == nullptr) {
        PR_ERR("Display not initialized");
        return OPRT_COM_ERROR;
    }

    if (x0 >= impl->width || y0 >= impl->height || x1 >= impl->width || y1 >= impl->height) {
        PR_ERR("Rectangle coordinates out of bounds");
        return OPRT_INVALID_PARM;
    }

    TDL_DISP_RECT_T rect = {
        .x0 = x0,
        .y0 = y0,
        .x1 = x1,
        .y1 = y1
    };

    // Convert RGB888 to display format
    uint32_t dispColor = tdl_disp_convert_color_fmt(color, TUYA_PIXEL_FMT_RGB888, 
                                                     impl->pixelFormat, 32768);

    OPERATE_RET rt = tdl_disp_draw_fill(impl->frameBuffer, &rect, dispColor, impl->isSwap);
    if (rt != OPRT_OK) {
        return rt;
    }
    
    return tdl_disp_dev_flush(impl->dispHandle, impl->frameBuffer);
}

OPERATE_RET Display::fillScreen(uint32_t color)
{
    return clear(color);
}

OPERATE_RET Display::drawImage(const uint16_t *imageData, uint16_t imgWidth, uint16_t imgHeight, 
                        uint16_t x, uint16_t y)
{
    if (_impl == nullptr) {
        PR_ERR("Display not initialized");
        return OPRT_COM_ERROR;
    }

    DisplayImpl *impl = static_cast<DisplayImpl*>(_impl);
    if (!impl->initialized) {
        PR_ERR("Display not initialized");
        return OPRT_COM_ERROR;
    }

    if (imageData == nullptr) {
        PR_ERR("Image data is NULL");
        return OPRT_INVALID_PARM;
    }

    // Check bounds
    if (x + imgWidth > impl->width || y + imgHeight > impl->height) {
        PR_ERR("Image exceeds display bounds");
        return OPRT_INVALID_PARM;
    }

    // Calculate image frame buffer size
    uint32_t frameLen = 0;
    if (impl->bitsPerPixel < 8) {
        frameLen = (imgWidth + impl->pixelsPerByte - 1) / impl->pixelsPerByte * imgHeight;
    } else {
        frameLen = imgWidth * imgHeight * impl->bytesPerPixel;
    }

    // Create image frame buffer
    TDL_DISP_FRAME_BUFF_T *imageFb = tdl_disp_create_frame_buff(DISP_FB_TP_PSRAM, frameLen);
    if (imageFb == nullptr) {
        PR_ERR("Failed to create image frame buffer");
        return OPRT_MALLOC_FAILED;
    }

    // Initialize frame buffer properties
    imageFb->x_start = x;
    imageFb->y_start = y;
    imageFb->fmt = impl->pixelFormat;
    imageFb->width = imgWidth;
    imageFb->height = imgHeight;

    // Convert and copy image data pixel by pixel
    for (uint32_t row = 0; row < imgHeight; row++) {
        for (uint32_t col = 0; col < imgWidth; col++) {
            uint16_t rgb565 = imageData[row * imgWidth + col];
            uint32_t color = tdl_disp_convert_rgb565_to_color(rgb565, impl->pixelFormat, 32768);
            tdl_disp_draw_point(imageFb, col, row, color, impl->isSwap);
        }
    }

    TDL_DISP_FRAME_BUFF_T *targetFb = imageFb;
    TDL_DISP_FRAME_BUFF_T *rotatedFb = nullptr;

    // Apply rotation if needed
    if (impl->devInfo.rotation != TUYA_DISPLAY_ROTATION_0) {
        // Calculate rotated frame buffer size
        uint32_t rotatedFrameLen = frameLen;
        uint16_t rotatedWidth = imgWidth;
        uint16_t rotatedHeight = imgHeight;
        
        // For 90 and 270 degree rotations, swap dimensions
        if (impl->devInfo.rotation == TUYA_DISPLAY_ROTATION_90 || 
            impl->devInfo.rotation == TUYA_DISPLAY_ROTATION_270) {
            rotatedWidth = imgHeight;
            rotatedHeight = imgWidth;
        }
        
        rotatedFb = tdl_disp_create_frame_buff(DISP_FB_TP_PSRAM, rotatedFrameLen);
        if (rotatedFb == nullptr) {
            PR_ERR("Failed to create rotated frame buffer");
            tdl_disp_free_frame_buff(imageFb);
            return OPRT_MALLOC_FAILED;
        }

        rotatedFb->x_start = x;
        rotatedFb->y_start = y;
        rotatedFb->fmt = imageFb->fmt;
        rotatedFb->width = rotatedWidth;
        rotatedFb->height = rotatedHeight;

        OPERATE_RET rt = tdl_disp_draw_rotate(impl->devInfo.rotation, imageFb, rotatedFb, impl->isSwap);
        if (rt != OPRT_OK) {
            PR_ERR("Failed to rotate image: %d", rt);
            tdl_disp_free_frame_buff(imageFb);
            tdl_disp_free_frame_buff(rotatedFb);
            return rt;
        }

        targetFb = rotatedFb;
    }

    // Flush to display
    OPERATE_RET rt = tdl_disp_dev_flush(impl->dispHandle, targetFb);
    
    // Clean up
    tdl_disp_free_frame_buff(imageFb);
    if (rotatedFb != nullptr) {
        tdl_disp_free_frame_buff(rotatedFb);
    }

    return rt;
}

OPERATE_RET Display::displayYUV422Frame(const uint8_t *yuvData, uint16_t width, uint16_t height)
{
    if (_impl == nullptr) {
        PR_ERR("Display not initialized");
        return OPRT_COM_ERROR;
    }

    DisplayImpl *impl = static_cast<DisplayImpl*>(_impl);
    if (!impl->initialized || impl->fbManage == nullptr) {
        PR_ERR("Display not initialized or frame buffer manager not available");
        return OPRT_COM_ERROR;
    }

    if (yuvData == nullptr) {
        PR_ERR("YUV data is NULL");
        return OPRT_INVALID_PARM;
    }

    // Get free frame buffer for conversion
    TDL_DISP_FRAME_BUFF_T *convertFb = tdl_disp_get_free_fb(impl->fbManage);
    if (convertFb == nullptr) {
        PR_ERR("No free frame buffer available");
        return OPRT_MALLOC_FAILED;
    }

    // Convert YUV422 to RGB565
    OPERATE_RET rt = tdl_disp_convert_yuv422_to_framebuffer((uint8_t*)yuvData, width, height, convertFb);
    if (rt != OPRT_OK) {
        PR_ERR("Failed to convert YUV422 to framebuffer: %d", rt);
        if (convertFb->free_cb) {
            convertFb->free_cb(convertFb);
        }
        return rt;
    }

    TDL_DISP_FRAME_BUFF_T *targetFb = convertFb;
    TDL_DISP_FRAME_BUFF_T *rotateFb = nullptr;

    // Handle rotation if needed
    if (impl->devInfo.rotation != TUYA_DISPLAY_ROTATION_0) {
        rotateFb = tdl_disp_get_free_fb(impl->fbManage);
        if (rotateFb == nullptr) {
            PR_ERR("No free frame buffer for rotation");
            if (convertFb->free_cb) {
                convertFb->free_cb(convertFb);
            }
            return OPRT_MALLOC_FAILED;
        }

        rt = tdl_disp_draw_rotate(impl->devInfo.rotation, convertFb, rotateFb, impl->isSwap);
        if (rt != OPRT_OK) {
            PR_ERR("Failed to rotate frame: %d", rt);
            if (convertFb->free_cb) {
                convertFb->free_cb(convertFb);
            }
            if (rotateFb->free_cb) {
                rotateFb->free_cb(rotateFb);
            }
            return rt;
        }

        targetFb = rotateFb;
        
        // Free convert buffer as we don't need it anymore
        if (convertFb->free_cb) {
            convertFb->free_cb(convertFb);
        }
    } else {
        // Swap RGB565 bytes if needed
        if (impl->isSwap) {
            tdl_disp_dev_rgb565_swap((uint16_t*)targetFb->frame, targetFb->width * targetFb->height);
        }
    }

    // Flush to display
    rt = tdl_disp_dev_flush(impl->dispHandle, targetFb);

    // Free frame buffer
    if (targetFb->free_cb) {
        targetFb->free_cb(targetFb);
    }

    return rt;
}

uint16_t Display::getWidth() const
{
    if (_impl == nullptr) {
        return 0;
    }
    return static_cast<DisplayImpl*>(_impl)->width;
}

uint16_t Display::getHeight() const
{
    if (_impl == nullptr) {
        return 0;
    }
    return static_cast<DisplayImpl*>(_impl)->height;
}

bool Display::isInitialized() const
{
    if (_impl == nullptr) {
        return false;
    }
    return static_cast<DisplayImpl*>(_impl)->initialized;
}

uint16_t Display::rgb888ToRgb565(uint8_t r, uint8_t g, uint8_t b)
{
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

uint32_t Display::rgb565ToRgb888(uint16_t rgb565)
{
    uint8_t r = (rgb565 >> 11) & 0x1F;
    uint8_t g = (rgb565 >> 5) & 0x3F;
    uint8_t b = rgb565 & 0x1F;
    
    // Scale to 8-bit
    r = (r << 3) | (r >> 2);
    g = (g << 2) | (g >> 4);
    b = (b << 3) | (b >> 2);
    
    return (r << 16) | (g << 8) | b;
}

OPERATE_RET Display::setRotation(Rotation rotation)
{
    if (_impl == nullptr) {
        PR_ERR("Display not initialized");
        return OPRT_COM_ERROR;
    }

    DisplayImpl *impl = static_cast<DisplayImpl*>(_impl);
    if (!impl->initialized) {
        PR_ERR("Display not initialized");
        return OPRT_COM_ERROR;
    }

    // Map Rotation enum to TUYA_DISPLAY_ROTATION_E
    TUYA_DISPLAY_ROTATION_E tuyaRotation;
    switch (rotation) {
        case Rotation::R0:
            tuyaRotation = TUYA_DISPLAY_ROTATION_0;
            break;
        case Rotation::R90:
            tuyaRotation = TUYA_DISPLAY_ROTATION_90;
            break;
        case Rotation::R180:
            tuyaRotation = TUYA_DISPLAY_ROTATION_180;
            break;
        case Rotation::R270:
            tuyaRotation = TUYA_DISPLAY_ROTATION_270;
            break;
        default:
            PR_ERR("Invalid rotation value");
            return OPRT_INVALID_PARM;
    }

    impl->devInfo.rotation = tuyaRotation;
    
    PR_DEBUG("Display rotation set to: %d, dimensions: %dx%d", (int)rotation, impl->width, impl->height);
    
    return OPRT_OK;
}

Display::Rotation Display::getRotation() const
{
    if (_impl == nullptr) {
        return Rotation::R0;
    }

    DisplayImpl *impl = static_cast<DisplayImpl*>(_impl);
    if (!impl->initialized) {
        return Rotation::R0;
    }

    // Map TUYA_DISPLAY_ROTATION_E back to Rotation enum
    switch (impl->devInfo.rotation) {
        case TUYA_DISPLAY_ROTATION_90:
            return Rotation::R90;
        case TUYA_DISPLAY_ROTATION_180:
            return Rotation::R180;
        case TUYA_DISPLAY_ROTATION_270:
            return Rotation::R270;
        case TUYA_DISPLAY_ROTATION_0:
        default:
            return Rotation::R0;
    }
}

void Display::swapRGB565Bytes(uint16_t *data, uint32_t len)
{
    if (data == nullptr || len == 0) {
        return;
    }

    tdl_disp_dev_rgb565_swap(data, len);
}

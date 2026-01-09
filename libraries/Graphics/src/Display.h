#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include "tuya_cloud_types.h"
#include "tal_api.h"

#include "tdl_display_manage.h"
#include "tdl_display_draw.h"
#include "tdl_display_driver.h"

#include "tuya_t5ai_ex_module.h"


class Display {
public:
    Display();
    ~Display();

    // Initialization and device management
    OPERATE_RET begin();
    void end();

    // Display control
    OPERATE_RET setBrightness(uint8_t brightness);
    OPERATE_RET flush();
    
    // Drawing operations
    OPERATE_RET clear(uint32_t color = 0x000000);
    OPERATE_RET drawPixel(uint16_t x, uint16_t y, uint32_t color);
    OPERATE_RET fillRect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint32_t color);
    OPERATE_RET fillScreen(uint32_t color);
    
    // Color conversion utilities
    uint32_t convertColor(uint32_t color, TUYA_DISPLAY_PIXEL_FMT_E srcFmt, TUYA_DISPLAY_PIXEL_FMT_E dstFmt, uint32_t threshold = 32768);
    uint32_t rgb565ToColor(uint16_t rgb565, uint32_t threshold = 32768);
    uint16_t rgb888ToRgb565(uint8_t r, uint8_t g, uint8_t b);
    
    // Image operations
    OPERATE_RET drawImage(const uint16_t *imageData, uint16_t imgWidth, uint16_t imgHeight, uint16_t x = 0, uint16_t y = 0);
    OPERATE_RET drawImageRotated(const uint16_t *imageData, uint16_t imgWidth, uint16_t imgHeight, TUYA_DISPLAY_ROTATION_E rotation);
    TDL_DISP_FRAME_BUFF_T* createImageBuffer(const uint16_t *imageData, uint16_t imgWidth, uint16_t imgHeight);
    int deleteImageBuffer(TDL_DISP_FRAME_BUFF_T *frameBuffer);
    OPERATE_RET flushFrameBuffer(TDL_DISP_FRAME_BUFF_T *frameBuffer);
    
    // Display information getters
    uint16_t getWidth() const { return _width; }
    uint16_t getHeight() const { return _height; }
    TUYA_DISPLAY_PIXEL_FMT_E getPixelFormat() const { return _pixelFormat; }
    TUYA_DISPLAY_ROTATION_E getRotation() const { return _devInfo.rotation; }
    bool isSwap() const { return _isSwap; }
    bool isInitialized() const { return _initialized; }

private:
    // Internal handles and state
    TDL_DISP_HANDLE_T _dispHandle;
    TDL_DISP_FRAME_BUFF_T *_frameBuffer;
    TDL_DISP_DEV_INFO_T _devInfo;
    
    // Display properties
    uint16_t _width;
    uint16_t _height;
    TUYA_DISPLAY_PIXEL_FMT_E _pixelFormat;
    bool _isSwap;
    bool _initialized;
    uint32_t _frameLength;
    uint8_t _bitsPerPixel;
    uint8_t _bytesPerPixel;
    uint8_t _pixelsPerByte;
};

#endif // __DISPLAY_H__
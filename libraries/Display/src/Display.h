/**
 * @file Display.h
 * @brief Arduino Display class for Tuya Open platform
 * @version 1.0
 * @copyright Copyright (c) 2021-2026 Tuya Inc. All Rights Reserved.
 */
#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include "tuya_cloud_types.h"
#include "board_com_api.h"
#include "tal_api.h"

class Display {
public:
    // Rotation options
    enum class Rotation { R0 = 0, R90 = 1, R180 = 2, R270 = 3 };

    Display();
    ~Display();

    // Initialization and device management
    OPERATE_RET begin();
    void end();

    // Display control
    OPERATE_RET setBrightness(uint8_t brightness);  // 0-100
    OPERATE_RET setRotation(Rotation rotation);
    Rotation getRotation() const;
    
    // Drawing operations
    OPERATE_RET clear(uint32_t color = 0x000000);
    OPERATE_RET drawPixel(uint16_t x, uint16_t y, uint32_t color);
    OPERATE_RET fillRect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint32_t color);
    OPERATE_RET fillScreen(uint32_t color);
    
    // Image operations (RGB565 format)
    OPERATE_RET drawImage(const uint16_t *imageData, uint16_t imgWidth, uint16_t imgHeight, 
                   uint16_t x = 0, uint16_t y = 0);
    
    // Camera frame display (YUV422 format)
    OPERATE_RET displayYUV422Frame(const uint8_t *yuvData, uint16_t width, uint16_t height);
    
    // Display information getters
    uint16_t getWidth() const;
    uint16_t getHeight() const;
    bool isInitialized() const;
    
    // Color conversion utilities
    static uint16_t rgb888ToRgb565(uint8_t r, uint8_t g, uint8_t b);
    static uint32_t rgb565ToRgb888(uint16_t rgb565);
    static void swapRGB565Bytes(uint16_t *data, uint32_t len);

private:
    // Internal implementation (opaque pointer)
    void *_impl;
};

#endif // __DISPLAY_H__
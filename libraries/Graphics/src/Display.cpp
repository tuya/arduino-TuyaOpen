#include "Display.h"
static char display_name[] = DISPLAY_NAME;
Display::Display() 
    : _dispHandle(NULL),
      _frameBuffer(NULL),
      _width(0),
      _height(0),
      _pixelFormat(TUYA_PIXEL_FMT_RGB565),
      _isSwap(false),
      _initialized(false),
      _frameLength(0),
      _bitsPerPixel(0),
      _bytesPerPixel(0),
      _pixelsPerByte(0)
{
    memset(&_devInfo, 0, sizeof(TDL_DISP_DEV_INFO_T));
    
#if defined(DISPLAY_NAME)
    // Register display hardware if configured
    OPERATE_RET rt = OPRT_OK;
    
#if defined(TUYA_T5AI_BOARD_EX_MODULE_35565LCD) && (TUYA_T5AI_BOARD_EX_MODULE_35565LCD == 1)
    DISP_RGB_DEVICE_CFG_T display_cfg;
    memset(&display_cfg, 0, sizeof(DISP_RGB_DEVICE_CFG_T));

    display_cfg.sw_spi_cfg.spi_clk = BOARD_LCD_SW_SPI_CLK_PIN;
    display_cfg.sw_spi_cfg.spi_sda = BOARD_LCD_SW_SPI_SDA_PIN;
    display_cfg.sw_spi_cfg.spi_csx = BOARD_LCD_SW_SPI_CSX_PIN;
    display_cfg.sw_spi_cfg.spi_dc  = BOARD_LCD_SW_SPI_DC_PIN;
    display_cfg.sw_spi_cfg.spi_rst = BOARD_LCD_SW_SPI_RST_PIN;

    display_cfg.bl.type              = BOARD_LCD_BL_TYPE;
    display_cfg.bl.pwm.id            = BOARD_LCD_BL_PWM_ID;
    display_cfg.bl.pwm.cfg.frequency = 1000;
    display_cfg.bl.pwm.cfg.duty      = 10000; // 0-10000

    display_cfg.width     = BOARD_LCD_WIDTH;
    display_cfg.height    = BOARD_LCD_HEIGHT;
    display_cfg.pixel_fmt = BOARD_LCD_PIXELS_FMT;
    display_cfg.rotation  = BOARD_LCD_ROTATION;
    display_cfg.power.pin = BOARD_LCD_POWER_PIN;

    rt = tdd_disp_rgb_ili9488_register(display_name, &display_cfg);
    if (rt != OPRT_OK) {
        PR_ERR("Failed to register ILI9488 display: %d", rt);
    }

    TDD_TP_GT1151_INFO_T tp_cfg = {
        .i2c_cfg = {
            .port = BOARD_TP_I2C_PORT,
            .scl_pin = BOARD_TP_I2C_SCL_PIN,
            .sda_pin = BOARD_TP_I2C_SDA_PIN,
        },
        .tp_cfg = {
            .x_max = BOARD_LCD_WIDTH,
            .y_max = BOARD_LCD_HEIGHT,
            .flags = {
                .swap_xy = 0,
                .mirror_x = 0,
                .mirror_y = 0,
            },
        },
    };
    rt = tdd_tp_i2c_gt1151_register(display_name, &tp_cfg);
    if (rt != OPRT_OK) {
        PR_ERR("Failed to register GT1151 touchpad: %d", rt);
    }
#endif
#endif
}

Display::~Display()
{
    end();
}

OPERATE_RET Display::begin()
{
    if (_initialized) {
        PR_WARN("Display already initialized");
        return OPRT_OK;
    }

#if !defined(DISPLAY_NAME)
    PR_ERR("DISPLAY_NAME not defined");
    return OPRT_INVALID_PARM;
#endif

    OPERATE_RET rt = OPRT_OK;
    
    // Find display device
    _dispHandle = tdl_disp_find_dev(display_name);
    if (_dispHandle == NULL) {
        PR_ERR("Display device %s not found", DISPLAY_NAME);
        return OPRT_COM_ERROR;
    }

    // Get display device information
    rt = tdl_disp_dev_get_info(_dispHandle, &_devInfo);
    if (rt != OPRT_OK) {
        PR_ERR("Failed to get display info: %d", rt);
        return rt;
    }

    // Store display properties
    _width = _devInfo.width;
    _height = _devInfo.height;
    _pixelFormat = _devInfo.fmt;
    _isSwap = _devInfo.is_swap;

    // Open display device
    rt = tdl_disp_dev_open(_dispHandle);
    if (rt != OPRT_OK) {
        PR_ERR("Failed to open display device: %d", rt);
        return rt;
    }

    // Set default brightness to 100%
    tdl_disp_set_brightness(_dispHandle, 100);

    // Calculate frame buffer size
    _bitsPerPixel = tdl_disp_get_fmt_bpp(_pixelFormat);
    if (_bitsPerPixel == 0) {
        PR_ERR("Unsupported pixel format: %d", _pixelFormat);
        tdl_disp_dev_close(_dispHandle);
        return OPRT_NOT_SUPPORTED;
    }

    if (_bitsPerPixel < 8) {
        _pixelsPerByte = 8 / _bitsPerPixel;
        _frameLength = (_width + _pixelsPerByte - 1) / _pixelsPerByte * _height;
    } else {
        _bytesPerPixel = (_bitsPerPixel + 7) / 8;
        _frameLength = _width * _height * _bytesPerPixel;
    }

    // Create frame buffer
    _frameBuffer = tdl_disp_create_frame_buff(DISP_FB_TP_PSRAM, _frameLength);
    if (_frameBuffer == NULL) {
        PR_ERR("Failed to create frame buffer");
        tdl_disp_dev_close(_dispHandle);
        return OPRT_MALLOC_FAILED;
    }

    // Initialize frame buffer properties
    _frameBuffer->x_start = 0;
    _frameBuffer->y_start = 0;
    _frameBuffer->fmt = _pixelFormat;
    _frameBuffer->width = _width;
    _frameBuffer->height = _height;

    _initialized = true;
    PR_DEBUG("Display initialized: %dx%d, fmt=%d, bpp=%d", _width, _height, _pixelFormat, _bitsPerPixel);
    
    return OPRT_OK;
}

void Display::end()
{
    if (!_initialized) {
        return;
    }

    if (_frameBuffer != NULL) {
        tdl_disp_free_frame_buff(_frameBuffer);
        _frameBuffer = NULL;
    }

    if (_dispHandle != NULL) {
        tdl_disp_dev_close(_dispHandle);
        _dispHandle = NULL;
    }

    _initialized = false;
    PR_DEBUG("Display closed");
}

OPERATE_RET Display::setBrightness(uint8_t brightness)
{
    if (!_initialized) {
        PR_ERR("Display not initialized");
        return OPRT_COM_ERROR;
    }

    return tdl_disp_set_brightness(_dispHandle, brightness);
}

OPERATE_RET Display::flush()
{
    if (!_initialized || _frameBuffer == NULL) {
        PR_ERR("Display not initialized or frame buffer is NULL");
        return OPRT_COM_ERROR;
    }

    return tdl_disp_dev_flush(_dispHandle, _frameBuffer);
}

OPERATE_RET Display::clear(uint32_t color)
{
    if (!_initialized || _frameBuffer == NULL) {
        PR_ERR("Display not initialized");
        return OPRT_COM_ERROR;
    }

    return tdl_disp_draw_fill_full(_frameBuffer, color, _isSwap);
}

OPERATE_RET Display::drawPixel(uint16_t x, uint16_t y, uint32_t color)
{
    if (!_initialized || _frameBuffer == NULL) {
        PR_ERR("Display not initialized");
        return OPRT_COM_ERROR;
    }

    if (x >= _width || y >= _height) {
        PR_ERR("Pixel coordinates out of bounds: (%d, %d)", x, y);
        return OPRT_INVALID_PARM;
    }

    return tdl_disp_draw_point(_frameBuffer, x, y, color, _isSwap);
}

OPERATE_RET Display::fillRect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint32_t color)
{
    if (!_initialized || _frameBuffer == NULL) {
        PR_ERR("Display not initialized");
        return OPRT_COM_ERROR;
    }

    if (x0 >= _width || y0 >= _height || x1 >= _width || y1 >= _height) {
        PR_ERR("Rectangle coordinates out of bounds");
        return OPRT_INVALID_PARM;
    }

    TDL_DISP_RECT_T rect = {
        .x0 = x0,
        .y0 = y0,
        .x1 = x1,
        .y1 = y1
    };

    return tdl_disp_draw_fill(_frameBuffer, &rect, color, _isSwap);
}

OPERATE_RET Display::fillScreen(uint32_t color)
{
    return clear(color);
}

uint32_t Display::convertColor(uint32_t color, TUYA_DISPLAY_PIXEL_FMT_E srcFmt, 
                               TUYA_DISPLAY_PIXEL_FMT_E dstFmt, uint32_t threshold)
{
    return tdl_disp_convert_color_fmt(color, srcFmt, dstFmt, threshold);
}

uint32_t Display::rgb565ToColor(uint16_t rgb565, uint32_t threshold)
{
    return tdl_disp_convert_rgb565_to_color(rgb565, _pixelFormat, threshold);
}

uint16_t Display::rgb888ToRgb565(uint8_t r, uint8_t g, uint8_t b)
{
    // Convert 8-8-8 RGB to 5-6-5 RGB565
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

TDL_DISP_FRAME_BUFF_T* Display::createImageBuffer(const uint16_t *imageData, uint16_t imgWidth, uint16_t imgHeight)
{
    if (!_initialized) {
        PR_ERR("Display not initialized");
        return NULL;
    }

    if (imageData == NULL || imgWidth == 0 || imgHeight == 0) {
        PR_ERR("Invalid image parameters");
        return NULL;
    }

    // Calculate frame buffer size
    uint32_t frameLen = 0;
    if (_bitsPerPixel < 8) {
        frameLen = (imgWidth + _pixelsPerByte - 1) / _pixelsPerByte * imgHeight;
    } else {
        frameLen = imgWidth * imgHeight * _bytesPerPixel;
    }

    // Create frame buffer
    TDL_DISP_FRAME_BUFF_T *fb = tdl_disp_create_frame_buff(DISP_FB_TP_PSRAM, frameLen);
    if (fb == NULL) {
        PR_ERR("Failed to create image frame buffer");
        return NULL;
    }

    // Initialize frame buffer properties
    fb->x_start = 0;
    fb->y_start = 0;
    fb->fmt = _pixelFormat;
    fb->width = imgWidth;
    fb->height = imgHeight;

    // Convert and copy image data pixel by pixel
    for (uint32_t y = 0; y < imgHeight; y++) {
        for (uint32_t x = 0; x < imgWidth; x++) {
            uint16_t rgb565Color = imageData[y * imgWidth + x];
            uint32_t displayColor = tdl_disp_convert_rgb565_to_color(rgb565Color, _pixelFormat, 0x1000);
            tdl_disp_draw_point(fb, x, y, displayColor, _isSwap);
        }
    }

    return fb;
}

int Display::deleteImageBuffer(TDL_DISP_FRAME_BUFF_T *frameBuffer)
{
    if (frameBuffer == NULL) {
        PR_ERR("Invalid frame buffer");
        return OPRT_INVALID_PARM;
    }
    tdl_disp_free_frame_buff(frameBuffer);
    return OPRT_OK;
}

OPERATE_RET Display::drawImage(const uint16_t *imageData, uint16_t imgWidth, uint16_t imgHeight, uint16_t x, uint16_t y)
{
    if (!_initialized || _frameBuffer == NULL) {
        PR_ERR("Display not initialized");
        return OPRT_COM_ERROR;
    }

    if (imageData == NULL) {
        PR_ERR("Image data is NULL");
        return OPRT_INVALID_PARM;
    }

    // Check bounds
    if (x + imgWidth > _width || y + imgHeight > _height) {
        PR_ERR("Image exceeds display bounds");
        return OPRT_INVALID_PARM;
    }

    // Draw image pixel by pixel to the main frame buffer
    for (uint32_t row = 0; row < imgHeight; row++) {
        for (uint32_t col = 0; col < imgWidth; col++) {
            uint16_t rgb565Color = imageData[row * imgWidth + col];
            uint32_t displayColor = tdl_disp_convert_rgb565_to_color(rgb565Color, _pixelFormat, 0x1000);
            tdl_disp_draw_point(_frameBuffer, x + col, y + row, displayColor, _isSwap);
        }
    }

    return OPRT_OK;
}

OPERATE_RET Display::drawImageRotated(const uint16_t *imageData, uint16_t imgWidth, uint16_t imgHeight, TUYA_DISPLAY_ROTATION_E rotation)
{
    if (!_initialized) {
        PR_ERR("Display not initialized");
        return OPRT_COM_ERROR;
    }

    if (imageData == NULL) {
        PR_ERR("Image data is NULL");
        return OPRT_INVALID_PARM;
    }

    // Create image frame buffer
    TDL_DISP_FRAME_BUFF_T *imageFb = createImageBuffer(imageData, imgWidth, imgHeight);
    if (imageFb == NULL) {
        return OPRT_MALLOC_FAILED;
    }

    TDL_DISP_FRAME_BUFF_T *targetFb = NULL;

    // Apply rotation if needed
    if (rotation != TUYA_DISPLAY_ROTATION_0) {
        TDL_DISP_FRAME_BUFF_T *rotatedFb = tdl_disp_create_frame_buff(DISP_FB_TP_PSRAM, imageFb->len);
        if (rotatedFb == NULL) {
            PR_ERR("Failed to create rotated frame buffer");
            tdl_disp_free_frame_buff(imageFb);
            return OPRT_MALLOC_FAILED;
        }

        rotatedFb->x_start = 0;
        rotatedFb->y_start = 0;
        rotatedFb->fmt = imageFb->fmt;

        OPERATE_RET rt = tdl_disp_draw_rotate(rotation, imageFb, rotatedFb, _isSwap);
        if (rt != OPRT_OK) {
            PR_ERR("Failed to rotate image: %d", rt);
            tdl_disp_free_frame_buff(imageFb);
            tdl_disp_free_frame_buff(rotatedFb);
            return rt;
        }

        targetFb = rotatedFb;
        tdl_disp_free_frame_buff(imageFb);
    } else {
        targetFb = imageFb;
    }

    // Flush to display
    OPERATE_RET rt = tdl_disp_dev_flush(_dispHandle, targetFb);
    
    // Clean up
    tdl_disp_free_frame_buff(targetFb);

    return rt;
}

OPERATE_RET Display::flushFrameBuffer(TDL_DISP_FRAME_BUFF_T *frameBuffer)
{
    if (!_initialized) {
        PR_ERR("Display not initialized");
        return OPRT_COM_ERROR;
    }

    if (frameBuffer == NULL) {
        PR_ERR("Frame buffer is NULL");
        return OPRT_INVALID_PARM;
    }

    return tdl_disp_dev_flush(_dispHandle, frameBuffer);
}
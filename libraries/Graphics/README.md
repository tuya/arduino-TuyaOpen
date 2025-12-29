# Graphics Library - Display C++ Wrapper

This library provides an object-oriented C++ interface to wrap the Tuya Display Layer (TDL) C display driver.

## Features

- **Device Management**: Initialize, find, and close display devices
- **Display Control**: Brightness adjustment, frame buffer refresh
- **Drawing Operations**: 
  - Draw single pixels
  - Fill rectangular areas
  - Clear screen / fill entire screen
- **Color Conversion**: RGB888 ↔ RGB565 ↔ other pixel formats
- **Device Information Query**: Width, height, pixel format

## API Reference

### Initialization and Device Management

```cpp
Display display;  // Create display object

OPERATE_RET begin();           // Initialize display device
void end();                     // Close display device
bool isInitialized() const;     // Check if initialized
```

### Display Control

```cpp
OPERATE_RET setBrightness(uint8_t brightness);  // Set brightness (0-100)
OPERATE_RET flush();                             // Flush frame buffer to display
```

### Drawing Operations

```cpp
// Clear display (default black)
OPERATE_RET clear(uint32_t color = 0x000000);

// Draw a single pixel
OPERATE_RET drawPixel(uint16_t x, uint16_t y, uint32_t color);

// Fill rectangular area from (x0,y0) to (x1,y1)
OPERATE_RET fillRect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint32_t color);

// Fill the entire screen
OPERATE_RET fillScreen(uint32_t color);
```

### Color Conversion

```cpp
// Convert color format
uint32_t convertColor(uint32_t color, 
                     TUYA_DISPLAY_PIXEL_FMT_E srcFmt, 
                     TUYA_DISPLAY_PIXEL_FMT_E dstFmt, 
                     uint32_t threshold = 32768);

// Convert RGB565 to current display format
uint32_t rgb565ToColor(uint16_t rgb565, uint32_t threshold = 32768);

// Convert RGB888 (8-8-8) to RGB565 (5-6-5)
uint16_t rgb888ToRgb565(uint8_t r, uint8_t g, uint8_t b);
```

### Information Query

```cpp
uint16_t getWidth() const;                          // Get display width
uint16_t getHeight() const;                         // Get display height
TUYA_DISPLAY_PIXEL_FMT_E getPixelFormat() const;   // Get pixel format
```

## Usage Examples

### Basic Example

```cpp
#include "display.h"

Display display;

void setup() {
    // Initialize display
    if (display.begin() != OPRT_OK) {
        Serial.println("Failed to initialize display");
        return;
    }
    
    // Clear screen (black)
    display.clear(0x000000);
    display.flush();
}

void loop() {
    // Fill screen with red
    display.fillScreen(0xFF0000);
    display.flush();
    delay(1000);
    
    // Fill screen with green
    display.fillScreen(0x00FF00);
    display.flush();
    delay(1000);
    
    // Fill screen with blue
    display.fillScreen(0x0000FF);
    display.flush();
    delay(1000);
}
```

### Drawing Shapes

```cpp
void setup() {
    display.begin();
    display.clear(0x000000);
    
    // Draw three colored rectangles
    display.fillRect(10, 10, 50, 50, 0xFF0000);   // Red
    display.fillRect(60, 10, 100, 50, 0x00FF00);  // Green
    display.fillRect(110, 10, 150, 50, 0x0000FF); // Blue
    
    display.flush();
}
```

### Pixel-Level Drawing

```cpp
void setup() {
    display.begin();
    display.clear(0x000000);
    
    // Draw a diagonal line
    uint16_t width = display.getWidth();
    uint16_t height = display.getHeight();
    
    for (int i = 0; i < min(width, height); i++) {
        display.drawPixel(i, i, 0xFFFFFF);  // White pixel
    }
    
    display.flush();
}
```

### Color Conversion

```cpp
void setup() {
    display.begin();
    
    // Create color using RGB888
    uint8_t r = 255, g = 128, b = 64;
    uint16_t rgb565 = display.rgb888ToRgb565(r, g, b);
    
    // Convert to display format
    uint32_t displayColor = display.rgb565ToColor(rgb565);
    
    // Fill screen
    display.fillScreen(displayColor);
    display.flush();
}
```

## Underlying TDL Interface Mapping

The Display class wraps the following TDL C interfaces:

| Display Method | TDL Interface |
|----------------|---------------|
| `begin()` | `tdl_disp_find_dev()`, `tdl_disp_dev_open()`, `tdl_disp_create_frame_buff()` |
| `end()` | `tdl_disp_dev_close()`, `tdl_disp_free_frame_buff()` |
| `setBrightness()` | `tdl_disp_set_brightness()` |
| `flush()` | `tdl_disp_dev_flush()` |
| `clear()` / `fillScreen()` | `tdl_disp_draw_fill_full()` |
| `drawPixel()` | `tdl_disp_draw_point()` |
| `fillRect()` | `tdl_disp_draw_fill()` |
| `convertColor()` | `tdl_disp_convert_color_fmt()` |
| `rgb565ToColor()` | `tdl_disp_convert_rgb565_to_color()` |

## Supported Display Types

Automatically supported based on board configuration:
- **ILI9488** - RGB interface LCD (TUYA_T5AI_BOARD_EX_MODULE_35565LCD)
- **ST7735S** - SPI interface LCD (TUYA_T5AI_BOARD_EX_MODULE_EYES)
- **ST7305** - E-Ink display (TUYA_T5AI_BOARD_EX_MODULE_29E_INK)
- **SSD1306** - OLED display (TUYA_T5AI_BOARD_EX_MODULE_096_OLED)

## Notes

1. **DISPLAY_NAME Macro**: Must be defined at compile time to specify the display device name to use
2. **Memory Allocation**: Frame buffer is automatically allocated from PSRAM, size calculated based on display resolution and pixel format
3. **Color Format**: Color values are automatically converted according to the display's pixel format (RGB565, RGB888, monochrome, etc.)
4. **Coordinate System**: 
   - Origin (0,0) is at the top-left corner
   - X-axis increases to the right
   - Y-axis increases downward
5. **Error Handling**: All operations return `OPERATE_RET`, `OPRT_OK` indicates success

## Performance Tips

- Call `flush()` once after batch drawing operations instead of refreshing after each draw
- Using `fillRect()` is more efficient than calling `drawPixel()` multiple times
- For intensive pixel operations, consider directly manipulating the frame buffer (advanced usage)

## Error Codes

| Return Value | Meaning |
|--------------|---------|
| `OPRT_OK` | Operation successful |
| `OPRT_INVALID_PARM` | Invalid parameter (e.g., coordinates out of bounds) |
| `OPRT_COM_ERROR` | Device not initialized or communication error |
| `OPRT_MALLOC_FAILED` | Memory allocation failed |
| `OPRT_NOT_SUPPORTED` | Unsupported pixel format |

# LVGL Demo Example

## Description

This example demonstrates how to use the LVGL v9.1.0 (Light and Versatile Graphics Library) with Tuya Open platform. LVGL is a popular open-source graphics library for creating beautiful user interfaces on embedded systems.

The example initializes LVGL, creates a simple "Hello World!" label centered on the screen with white background, and starts the LVGL task handler.

More about LVGL can be found at [LVGL official website](https://lvgl.io/).

## Hardware Requirements

- Tuya Open development board with display support
- Compatible LCD/OLED display module
- Sufficient RAM for LVGL operations

## Usage Instructions

1. Connect your display module to the development board according to your hardware specifications.

2. Upload the sketch to your board.

3. Open the Serial Monitor (115200 baud) to view initialization logs.

4. The display will show "Hello World!" text centered on a white background.

5. The LVGL task runs continuously to handle graphics updates and animations.

## Key Features

- **LVGL Integration**: Full integration with LVGL graphics library
- **Hardware Registration**: Initializes display hardware using board API
- **Display Locking**: Implements thread-safe display operations with lock/unlock
- **Custom Styling**: Demonstrates custom background and text colors
- **Task Management**: Starts LVGL task with configurable period (5ms) and stack size (8KB)
- **Label Creation**: Shows basic LVGL object creation and alignment

## Code Highlights

- Registers hardware using `board_register_hardware()`
- Initializes LVGL vendor layer with display name: `lv_vendor_init()`
- Uses display locking mechanism for thread safety: `lv_vendor_disp_lock()` / `lv_vendor_disp_unlock()`
- Sets active screen background to white: `lv_obj_set_style_bg_color(lv_screen_active(), lv_color_white(), LV_PART_MAIN)`
- Creates and centers a label with black text
- Starts LVGL task handler: `lv_vendor_start(5, 1024*8)` with 5ms period and 8KB stack
- Minimal loop() implementation as LVGL handles updates in its own task

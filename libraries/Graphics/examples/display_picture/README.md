# Display Picture Example

## Description

This example demonstrates how to display an image on the screen using the Graphics Display library. It shows how to handle image data, manage display rotation, and efficiently render images to the display.

The example supports both rotated and non-rotated image display, automatically selecting the appropriate method based on the display's rotation setting.

## Hardware Requirements

- Tuya Open development board with display support
- Compatible LCD/OLED display module

## Usage Instructions

1. Connect your display module to the development board.

2. Ensure you have an image data file (typically included as external data arrays).

3. Upload the sketch to your board.

4. Open the Serial Monitor (115200 baud) to view system information and debug messages.

5. The display will show the image and refresh it every 5 seconds.

## Key Features

- **Image Display**: Renders pre-loaded image data to the screen
- **Rotation Support**: Automatically handles different display rotation angles (0°, 90°, 180°, 270°)
- **System Information**: Displays project details and platform info
- **Brightness Control**: Sets display brightness to 100%
- **Efficient Rendering**: Uses optimized methods for rotated and non-rotated images
- **Frame Buffer Management**: Demonstrates proper image buffer creation and cleanup

## Code Highlights

- Uses external image data arrays (`imga_data`, `imga_width`, `imga_height`)
- Checks display rotation and selects appropriate rendering method:
  - For rotated displays: Uses `drawImageRotated()` function
  - For non-rotated displays: Creates frame buffer with `createImageBuffer()` and flushes directly
- Implements proper memory management with buffer cleanup
- Displays detailed system information including image dimensions
- Updates display every 5 seconds
- Includes comprehensive error handling for all display operations

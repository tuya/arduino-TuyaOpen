# Display Fill Example

## Description

This example demonstrates basic display functionality by filling the entire screen with random colors. It shows how to initialize the Graphics Display library, clear the screen, and continuously update it with randomly generated colors.

The example is useful for testing display hardware and understanding basic display operations.

## Hardware Requirements

- Tuya Open development board with display support
- Compatible LCD/OLED display module

## Usage Instructions

1. Connect your display module to the development board according to your hardware specifications.

2. Upload the sketch to your board.

3. Open the Serial Monitor (115200 baud) to view system information and debug messages.

4. Observe the display as it cycles through random colors every 2 seconds.

## Key Features

- **Display Initialization**: Properly initializes the display hardware
- **System Information**: Prints project details, version, and platform info to serial
- **Screen Clearing**: Demonstrates how to clear the screen with a specific color
- **Random Color Fill**: Fills the entire screen with random colors
- **Display Flushing**: Shows proper buffer flushing to update the display
- **Error Handling**: Includes error checking for display operations

## Code Highlights

- Uses the `Display` class from the Graphics library
- Displays system information including project name, version, compile time, and platform details
- Retrieves and displays screen resolution and pixel format information
- Generates random colors using `tal_system_get_random()`
- Implements proper error handling with `OPERATE_RET` return codes
- Updates display every 2 seconds with a new random color

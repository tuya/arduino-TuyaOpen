# Log Output Example

## Description

This example demonstrates the basic usage of the Log library on Tuya boards. It shows how to initialize the logging system and output debug messages to the serial monitor.

## Hardware Requirements

- Any Tuya-supported development board (T2, T3, T5, ESP32, LN882H, or XH_WB5E)
- USB cable for serial connection

## Usage Instructions

1. Open the example in Arduino IDE
2. Select your Tuya board from Tools > Board menu
3. Upload the sketch to your board
4. Open Serial Monitor at 115200 baud rate
5. Observe "Hello" messages being printed every second

## Key Features

- **Log Initialization**: Initializes the log system with a 1024-byte buffer
- **Debug Output**: Uses `PR_DEBUG` macro to output debug-level messages
- **Simple Logging**: Demonstrates the simplest way to integrate logging into your project

## Code Explanation

- `Log.begin(1024)`: Initializes the logging system with a 1024-byte buffer
- `PR_DEBUG("Hello")`: Outputs a debug-level log message with the text "Hello"
- The message is printed every second in the main loop

## Output

The serial monitor will continuously display:
```
Hello
Hello
Hello
...
```

## Notes

This example uses the debug log level. The Log library typically supports multiple log levels (ERROR, WARNING, INFO, DEBUG, etc.) for different types of messages.

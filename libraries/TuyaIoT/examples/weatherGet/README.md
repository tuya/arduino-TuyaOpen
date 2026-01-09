# Weather Get Example

## Overview

This example demonstrates how to integrate Tuya IoT Weather Service with your device. It shows how to retrieve weather data from Tuya cloud and control a smart switch with LED indicator.

## Features

- Weather data retrieval from Tuya IoT Weather Service
- Smart switch functionality with LED control
- Button-based user interaction
- LED status indication (off, on, blinking)
- Network configuration management
- Memory monitoring via Ticker

## Hardware Requirements

- Tuya-supported development board
- LED connected to LED_BUILTIN pin
- Button connected to BUTTON_BUILTIN pin

## Configuration

Configure these parameters before uploading:

```cpp
#define TUYA_DEVICE_UUID "uuidxxxxxxxxxxxxxxxx"      // Your device UUID
#define TUYA_DEVICE_AUTHKEY "xxxxxxxxxxxxxxxxxxxxxxxx" // Your auth key
```

Product ID: `qhivvyqawogv04e4`

## Pin Configuration

- `LED_BUILTIN`: Status LED (turns on with LOW level)
- `BUTTON_BUILTIN`: User button (active LOW)

## How It Works

1. **Initialization**: Sets up logging, LED, button, and IoT connection
2. **Weather Service**: Initializes TuyaIoTWeather service
3. **LED Control**: Manages LED states based on switch DP
4. **Button Handling**: Short press toggles switch, long press removes device
5. **Weather Demo**: Demonstrates weather data retrieval
6. **Heap Monitor**: Reports free heap every 5 seconds

## Data Points

- `DPID_SWITCH` (1): Boolean - Switch state (controls LED)

## Button Controls

- **Short Press**: Toggle switch/LED state
- **Long Press (3s)**: Remove device from Tuya IoT platform

## LED States

- **Off**: Switch is off
- **On**: Switch is on
- **Blinking (500ms)**: Device is in binding mode

## Usage

1. Flash firmware to your device
2. Open Serial Monitor at 115200 baud
3. Device connects to Tuya IoT platform
4. Use button to control LED or use Tuya Smart app
5. Weather data will be retrieved automatically

## Weather Service

The example includes `weatherGetDemo()` function to demonstrate weather data retrieval. You can customize this function to:
- Get current weather
- Get weather forecast
- Get temperature, humidity, etc.
- Display weather information

## Events Handled

- `TUYA_EVENT_BIND_START`: Device enters binding mode
- `TUYA_EVENT_ACTIVATE_SUCCESSED`: Device activation complete
- `TUYA_EVENT_MQTT_CONNECTED`: Connected to Tuya cloud
- `TUYA_EVENT_TIMESTAMP_SYNC`: Time synchronization
- `TUYA_EVENT_DP_RECEIVE_OBJ`: Received DP commands

## Dependencies

- TuyaIoT library
- TuyaIoTWeather library
- tLed library
- Log library
- Ticker library

## Notes

- Ensure valid Tuya device license
- Weather service requires device to be online
- Free heap monitoring helps track memory usage
- The tLed class provides easy LED control with blinking

## Troubleshooting

- Check UUID and AuthKey if connection fails
- Verify network configuration
- Monitor serial output for errors
- Check free heap if device behaves unexpectedly

## Advanced Features

- Customize weather data retrieval
- Add more DPs for additional controls
- Implement weather-based automation
- Store weather history

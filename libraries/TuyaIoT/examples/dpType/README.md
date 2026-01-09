# TuyaIoT dpType Example

## Overview

This example demonstrates how to handle different Data Point (DP) types in Tuya IoT applications. It shows how to read and write various DP types including boolean, value, enum, string, bitmap, and raw data types.

## Features

- Handle multiple DP types (boolean, value, enum, string, bitmap, raw)
- Read DP values from Tuya cloud
- Write DP values to Tuya cloud
- Button control for device reset
- Network configuration management

## Hardware Requirements

- Tuya-supported development board (ESP32, T2, T3, T5, etc.)
- Button connected to BUTTON_BUILTIN pin

## Data Points

The example defines the following DPs:

- `DPID_SWITCH` (20): Boolean type - Switch state
- `DPID_MODE` (21): Enum type - Operating mode
- `DPID_BRIGHT` (22): Value type - Brightness level
- `DPID_BITMAP` (101): Bitmap type - Multiple flags
- `DPID_STRING` (102): String type - Text data
- `DPID_RAW` (103): Raw type - Binary data

## Configuration

Before uploading, configure these parameters:

```cpp
#define TUYA_DEVICE_UUID "uuidxxxxxxxxxxxxxxxx"      // Replace with your device UUID
#define TUYA_DEVICE_AUTHKEY "xxxxxxxxxxxxxxxxxxxxxxxx" // Replace with your auth key
```

Product ID: `2avicuxv6zgeiquf` (configured in code)

## How It Works

1. **Initialization**: Initializes serial communication and logging
2. **License Setup**: Reads board license or uses hardcoded credentials
3. **IoT Connection**: Connects to Tuya IoT platform
4. **DP Handling**: Receives DP commands from cloud and reports status back
5. **Button Control**: Long press button to remove device

## DP Operations

### Boolean (Switch)
```cpp
bool switchStatus = 0;
TuyaIoT.read(event, DPID_SWITCH, switchStatus);
TuyaIoT.write(DPID_SWITCH, switchStatus);
```

### Value (Integer)
```cpp
int brightValue = 0;
TuyaIoT.read(event, DPID_BRIGHT, brightValue);
TuyaIoT.write(DPID_BRIGHT, brightValue);
```

### Enum (Mode)
```cpp
uint32_t mode = 0;
TuyaIoT.read(event, DPID_MODE, mode);
TuyaIoT.write(DPID_MODE, mode);
```

### String
```cpp
char *strValue = NULL;
TuyaIoT.read(event, DPID_STRING, strValue);
TuyaIoT.write(DPID_STRING, strValue);
```

### Raw (Binary)
```cpp
uint8_t *rawValue = NULL;
uint16_t len = 0;
TuyaIoT.read(event, DPID_RAW, rawValue, len);
TuyaIoT.write(DPID_RAW, rawValue, len);
```

## Usage

1. Flash the firmware to your device
2. Open Serial Monitor at 115200 baud
3. Device will connect to Tuya IoT platform
4. Use Tuya Smart app to control DPs
5. Monitor serial output to see DP values
6. Long press button (3 seconds) to remove device

## Button Controls

- **Short Press**: No action (can be customized)
- **Long Press (3s)**: Remove device from Tuya IoT platform

## Events Handled

- `TUYA_EVENT_BIND_START`: Device binding started
- `TUYA_EVENT_ACTIVATE_SUCCESSED`: Device activation succeeded
- `TUYA_EVENT_TIMESTAMP_SYNC`: Time synchronization
- `TUYA_EVENT_DP_RECEIVE_OBJ`: Received object-type DP data
- `TUYA_EVENT_DP_RECEIVE_RAW`: Received raw-type DP data

## Dependencies

- TuyaIoT library
- Log library

## Notes

- Ensure you have a valid Tuya device license
- Configure the correct Product ID for your device type
- The example demonstrates all common DP types
- DP IDs must match your product schema in Tuya IoT Platform

## Troubleshooting

- If device doesn't connect, verify UUID and AuthKey
- Check Product ID matches your product on Tuya platform
- Ensure DP IDs match your product schema
- Check serial output for detailed logs

## Learn More

- Understand how to define DPs in Tuya IoT Platform
- Learn about different DP types and their use cases
- See how to handle bidirectional DP communication

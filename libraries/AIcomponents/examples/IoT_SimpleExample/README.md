# Switch Demo

## Overview

This example demonstrates a basic IoT switch implementation using the Tuya IoT platform. It showcases how to create a simple controllable device that can be managed through the Tuya Smart app, controlling an LED based on Data Point (DP) commands received from the cloud.

## Features

- **Simple Switch Control**: Control LED via Tuya Smart app
- **Tuya IoT Integration**: Full cloud connectivity and device binding
- **Data Point Processing**: Handles multiple DP types (Bool, Value, String, Enum, Bitmap)
- **LED Indicator**: Visual feedback through LED control (GPIO 1)
- **Device License Management**: Supports both board license and manual configuration
- **Serial Communication**: Real-time serial monitor for debugging

## Hardware Requirements

- Tuya-compatible development board (ESP32, T2, T3, T5, LN882H, or XH_WB5E series)
- LED connected to GPIO 1 (for switch status indication)
- Stable Wi-Fi connection
- Serial console for monitoring (baud rate 115200)

## Configuration

Before uploading the code, configure the following parameters:

```cpp
#define TUYA_DEVICE_UUID "uuidxxxxxxxxxxxxxxxx"
#define TUYA_DEVICE_AUTHKEY "keyxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
#define TUYA_PRODUCT_ID "alon7qgyjj8yus74"
```

You can obtain a PID by creating a product on the [Tuya Developer Platform](https://tuyaopen.ai/en/docs/cloud/tuya-cloud/creating-new-product) or use the default PID.

For information on obtaining authorization codes, refer to [Authorization Code Acquisition](https://tuyaopen.ai/en/docs/quick-start#tuyaopen-authorization-code-acquisition).
You must replace with your own device UUID and AUTHKEY.

The example will automatically attempt to read credentials from the board license. If not available, it will use the values defined above.

## Usage

1. **Upload the Code**: Flash the code to your development board
2. **Open Serial Monitor**: Set baud rate to 115200 to view debug information
3. **Device Binding**: 
   - Power on the device
   - Open [Tuya Smart app](https://tuyaopen.ai/en/docs/quick-start/device-network-configuration)
   - Add a new device and follow pairing instructions
4. **Control the Switch**:
   - Use the Tuya Smart app to toggle the switch state
   - LED on GPIO 1 will turn ON/OFF accordingly
   - Monitor events in Serial Monitor

### LED Behavior

- **LED OFF (HIGH)**: Switch is OFF
- **LED ON (LOW)**: Switch is ON

## Key Functions

### Main Components

- **`setup()`**: Initializes serial communication, GPIO pins, and Tuya IoT client
- **`loop()`**: Maintains serial communication and checks for incoming data
- **`tuyaIoTEventCallback()`**: Handles all IoT platform events

### Event Handling

The callback function processes various Tuya IoT events:

#### Device Lifecycle Events
- **`TUYA_EVENT_BIND_START`**: Device pairing started
- **`TUYA_EVENT_DIRECT_MQTT_CONNECTED`**: Direct MQTT connection established (displays QR code if enabled)
- **`TUYA_EVENT_MQTT_CONNECTED`**: Cloud connection successful, device is online
- **`TUYA_EVENT_RESET`**: Device reset requested

#### Data Point Events
- **`TUYA_EVENT_DP_RECEIVE_OBJ`**: Receives object-type data points
  - **PROP_BOOL**: Boolean values (switch ON/OFF)
  - **PROP_VALUE**: Integer values
  - **PROP_STR**: String values
  - **PROP_ENUM**: Enumeration values
  - **PROP_BITMAP**: Bitmap flags
- **`TUYA_EVENT_DP_RECEIVE_RAW`**: Receives raw data points

#### Other Events
- **`TUYA_EVENT_UPGRADE_NOTIFY`**: OTA upgrade notification
- **`TUYA_EVENT_TIMESTAMP_SYNC`**: Time synchronization with cloud

### Data Point Processing

When a boolean DP is received:
```cpp
if (dp->value.dp_bool == true) {
    digitalWrite(APP_LED_PIN, LOW);  // LED ON
} else {
    digitalWrite(APP_LED_PIN, HIGH); // LED OFF
}
```

After processing, the DP value is reported back to the cloud for status synchronization.

## Project Structure

```
switch_demo/
├── switch_demo.ino    # Main program file
└── README.md          # This documentation
```

## Troubleshooting

- **Device not connecting**: 
  - Verify Wi-Fi credentials in network configuration
  - Check that Tuya account is properly set up
  - Ensure device UUID and AUTHKEY are correct

- **LED not responding**: 
  - Check GPIO 1 connection
  - Verify LED is properly connected
  - Monitor Serial output to confirm DP reception

## Notes

- The device will automatically reset network configuration if the reset button is pressed three times
- LED on GPIO 1 indicates device status
- This example requires a valid Tuya IoT account and properly configured product

## Related Documentation

 - [TuyaOpen Official Website](https://tuyaopen.ai)
 - [Github Repository](https://github.com/tuya/TuyaOpen)
 - [Arduino-TuyaOpen Repository](https://github.com/tuya/Arduino-TuyaOpen)
 - [Device Network Configuration](https://tuyaopen.ai/en/docs/quick-start/device-network-configuration) (requires APP download)

## Contact Us

If you have any good suggestions and feedback, please submit an issue on [Github](https://github.com/tuya/TuyaOpen/issues) !!!

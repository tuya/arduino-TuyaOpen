# MQTT Authentication Example

## Description

This example demonstrates how to connect to an MQTT broker with authentication (username and password). It shows the basic setup for establishing a secure MQTT connection, publishing messages, and subscribing to topics using the MQTTClient library on Tuya boards.

## Hardware Requirements

- Any Tuya-supported development board with WiFi capability (T2, T3, T5, ESP32, or XH_WB5E)
- WiFi network connection
- Access to an MQTT broker (example uses broker.emqx.io)

## Usage Instructions

1. Open the example in Arduino IDE
2. Modify the WiFi credentials:
   ```cpp
   const char *ssid = "your_ssid";
   const char *pass = "your_passwd";
   ```
3. Optionally modify the MQTT settings:
   - `mqtt_broker`: MQTT broker address
   - `topic`: Topic to publish/subscribe
   - `mqtt_username`: MQTT username
   - `mqtt_password`: MQTT password
4. Select your Tuya board from Tools > Board menu
5. Upload the sketch to your board
6. Open Serial Monitor at 115200 baud rate
7. Watch the board connect to WiFi and MQTT broker

## Key Features

- **WiFi Connection**: Connects to WiFi network and displays IP address
- **MQTT Authentication**: Demonstrates connecting to MQTT broker with username and password
- **Client Configuration**: Shows how to configure MQTT client parameters:
  - Host and port
  - Keep-alive interval
  - Timeout settings
  - Client ID
- **Publishing**: Publishes a message to the specified topic
- **Subscribing**: Subscribes to a topic to receive messages
- **Message Callback**: Includes callback function to handle incoming messages

## Configuration Parameters

- **Keep-alive**: 15 seconds - connection heartbeat interval
- **Timeout**: 100 ms - operation timeout
- **Port**: 1883 - standard MQTT port (unencrypted)

## Notes

- The example uses the public MQTT broker `broker.emqx.io` which is suitable for testing
- For production applications, use your own MQTT broker
- The example shows authentication setup, but the broker used may not require it
- Implement the `callback` function to handle received messages

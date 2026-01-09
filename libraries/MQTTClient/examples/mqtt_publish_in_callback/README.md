# MQTT Publish in Callback Example

## Description

This example demonstrates how to publish MQTT messages from within a callback function. When a message is received on the "inTopic" subscription, it is automatically republished to "outTopic". This pattern is useful for creating message relay systems, data forwarding, or implementing simple MQTT-based communication protocols.

## Hardware Requirements

- Any Tuya-supported development board with WiFi capability (T2, T3, T5, ESP32, or XH_WB5E)
- WiFi network connection
- Access to an MQTT broker (example uses broker.emqx.io)

## Usage Instructions

1. Open the example in Arduino IDE
2. Modify the WiFi credentials:
   ```cpp
   const char* ssid = "your_ssid";
   const char* password = "your_passwd";
   ```
3. Optionally modify the MQTT broker settings:
   - `mqtt_server`: MQTT broker address
   - `mqtt_username`: MQTT username
   - `mqtt_password`: MQTT password
4. Select your Tuya board from Tools > Board menu
5. Upload the sketch to your board
6. Open Serial Monitor at 115200 baud rate
7. Send a message to "inTopic" and observe it being republished to "youroutTopic"

## Key Features

- **WiFi Connection**: Connects to WiFi with connection status display
- **MQTT Reconnection**: Automatically reconnects to MQTT broker if connection is lost
- **Message Forwarding**: Receives messages and republishes them to a different topic
- **Callback Publishing**: Demonstrates safe message publishing within callback function
- **Debug Logging**: Uses Log library for debug-level logging (set to DEBUG level)

## Callback Function

The `mqtt_message_cb` callback function:
1. Receives a message on the subscribed topic ("inTopic")
2. Extracts the message payload
3. Prints the message to Serial Monitor
4. Republishes the same message to "youroutTopic"

## Configuration Parameters

- **Host**: broker.emqx.io (public test broker)
- **Port**: 1883 (standard MQTT)
- **Keep-alive**: 120 seconds (longer than basic example for stability)
- **Timeout**: 8000 ms (8 seconds - longer for reliability)
- **Client ID**: ty_test_id

## Important Design Notes

- **Global Client Reference**: The client pointer `cli` must be declared globally (before the callback) to ensure it's accessible within the callback function
- **Non-blocking Reconnection**: The loop checks connection status and reconnects if needed
- **Initialization Sequence**: The callback function must be set before initializing the client

## Output Example

When connecting:
```
Connecting to your_ssid
...
WiFi connected
IP address: 
192.168.1.100
```

When receiving and forwarding messages:
```
message: Hello from inTopic
(Message automatically republished to youroutTopic)
```

## Use Cases

- Message routing between different MQTT topics
- Data forwarding and relay applications
- Building MQTT bridges
- Implementing publish-on-receive patterns
- Creating notification systems

## Notes

- This example shows the correct way to publish from within a callback
- The client reference must be valid when used in the callback
- Consider rate limiting when republishing to avoid message loops
- For production use, add error handling and connection monitoring

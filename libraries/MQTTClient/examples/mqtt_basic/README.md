# MQTT Basic Example

## Description

This example demonstrates the fundamental capabilities of the MQTTClient library. It connects to an MQTT server, publishes messages to a topic, subscribes to another topic, and handles incoming messages. This is a comprehensive basic example showing typical MQTT operations on Tuya boards.

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
   - `intopic`: Topic to subscribe to (receives messages)
   - `outtopic`: Topic to publish to (sends messages)
   - `mqtt_username`: MQTT username
   - `mqtt_password`: MQTT password
4. Select your Tuya board from Tools > Board menu
5. Upload the sketch to your board
6. Open Serial Monitor at 115200 baud rate
7. Observe WiFi connection and MQTT operations

## Key Features

- **WiFi Connection**: Establishes WiFi connection with status monitoring
- **MQTT Client Setup**: Configures MQTT client with essential parameters
- **Publishing**: Publishes test message to the output topic
- **Subscribing**: Subscribes to input topic to receive messages
- **Message Handling**: Implements callback to process incoming messages
- **Continuous Operation**: Maintains MQTT connection with `mqtt.loop()` in the main loop

## Message Callback

The `mqtt_message_cb` function handles received messages and displays:
- Topic name
- Payload length
- Message content (as string)

## Configuration Parameters

- **Host**: broker.emqx.io (public test broker)
- **Port**: 1883 (standard MQTT)
- **Keep-alive**: 15 seconds
- **Timeout**: 100 ms
- **Client ID**: ty_test_id

## Output Example

When the sketch runs, you'll see:
```
...
WiFi connected
IP address: 
192.168.1.100
wifi connected
```

When messages are received:
```
recv message TopicName: inTopic
payload len: 13
message: Hello World!
```

## Notes

- This example uses a blocking WiFi connection method
- The `mqtt.loop()` call in the loop() function is essential for maintaining the connection and processing messages
- Messages are assumed to be strings, not binary data
- For production use, implement reconnection logic if the connection is lost

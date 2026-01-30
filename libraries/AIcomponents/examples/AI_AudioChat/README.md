# AI Audio Example

## Overview

This example demonstrates how to integrate AI audio capabilities into IoT devices using the Tuya IoT platform. It provides text-based interaction with an AI agent through serial input, allowing users to chat with the AI and receive responses in real-time.

## Wake Words

 - **你好涂鸦** (Ni Hao Tuya)
 - **Hey Tuya**

## Features

- **Text Input Interface**: Chat with AI agent via Serial Monitor
- **AI Audio Integration**: Utilizes TuyaAI library for audio processing
- **IoT Connectivity**: Full integration with Tuya IoT cloud platform
- **Event-Driven Architecture**: Dynamically handles audio states and events
- **Network Configuration**: Supports device binding and configuration reset
- **Data Point Processing**: Handles both object and raw DP types

## Hardware Requirements

- Tuya-compatible development board (`TUYA_T5AI`)
- LED connected to GPIO 1 (optional, for status indication)
- Button connected to GPIO 12 (optional, for reset functionality)
- Stable Wi-Fi connection
- Serial console for interaction (baud rate 115200)

The `TUYA_T5AI` development board is recommended for use with the [T5AI-Board/T5AI-Core Development Kit](https://tuyaopen.ai/en/docs/hardware-specific/t5-ai-board/overview-t5-ai-board).    

## Configuration

Before uploading the code, configure the following parameters:

```cpp
#define TUYA_DEVICE_UUID "uuidxxxxxxxxxxxxxxxx"
#define TUYA_DEVICE_AUTHKEY "keyxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
#define TUYA_PRODUCT_ID "9inb01mvjqh5zhhr"
```

You can obtain a PID by creating a product on the [Tuya Developer Platform](https://tuyaopen.ai/en/docs/cloud/tuya-cloud/creating-new-product) or use the default PID.

For information on obtaining authorization codes, refer to [Authorization Code Acquisition](https://tuyaopen.ai/en/docs/quick-start#tuyaopen-authorization-code-acquisition).
You must replace with your own device UUID and AUTHKEY.

## Usage

1. **Upload the Code**: Upload the code to your development board
2. **Open Serial Monitor**: Set baud rate to 115200
3. **Device Initialization**: Wait for the device to connect to Wi-Fi and Tuya cloud
4. **Start Chatting**: Type messages in the Serial Monitor and press Enter
5. **AI Responses**: The AI will respond to your queries in real-time

### Serial Monitor Interaction Example

```
🧍‍♂️ User: Hello, how are you doing?
AI: I'm doing great! Is there anything I can help you with?
================chat end=================
```

## Key Functions

### Main Components

- **`setup()`**: Initializes hardware, logging, Tuya IoT connection, and AI audio components
- **`loop()`**: Handles continuous user input from Serial Monitor
- **`handleUserInput()`**: Processes text input and sends it to the AI agent
- **`tuyaIoTEventCallback()`**: Manages IoT events (binding, MQTT connection, DP reception, etc.)
- **`aiEventCallback()`**: Processes AI audio events (ASR text, AI replies, emotions, wake-up)
- **`aiStateCallback()`**: Tracks AI audio state transitions

### AI Audio Work Mode

This example uses `WAKE_UP` mode, which combines:
- Automatic Speech Recognition (ASR)
- Wake word detection
- Single-turn conversation handling

### Event Handling

The code responds to various events:
- **Device Binding**: Handles initial pairing with Tuya app
- **MQTT Connection**: Manages cloud connection status
- **Data Points**: Processes commands from cloud or app
- **AI Text Streaming**: Receives and displays AI responses in real-time
- **Emotions**: Processes emotion data in AI responses

## Troubleshooting

- **Device not connecting**: Check Wi-Fi credentials and ensure network access is normal
- **No AI response**: Verify device is bound to Tuya cloud and MQTT is connected
- **Serial input not working**: Ensure Serial Monitor is set to 115200 baud rate and uses newline terminator

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

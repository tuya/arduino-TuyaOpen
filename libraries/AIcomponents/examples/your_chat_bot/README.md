# Your chatbot

## Overview

This is a complete example of an AI voice assistant, combining voice interaction, integration of displays, and IoT connectivity. It demonstrates the complete functionality of using the Tuya IoT platform to implement an AI voice assistant, including button control, LED indicators, display output, and text and voice interaction functions.

## Functional characteristics

*   Multi-mode interaction: Supports voice (ASR) and text input.
*   Button Control: Physical buttons are used for waking up and controlling the dialogue.
*   Display Integration: Selectable screen display of chat messages and system status.
*   Volume control: Adjust audio volume through cloud commands.
*   **State Visualization**: Real-time status and emotional display
*   **LED indicator lights**：Device status visual feedback
*   Network configuration: Automatic configuration and support for QR codes.
*   **AI audio streaming**: Real-time streaming of AI responses
*   Strong word detection: Use strong words to activate without manual activation.

## Hardware requirements

*   Compatible development board for TUYA_T5AI.
*   Connect to GPIO 1's LED (used for status indication)
*   Connect to the button on GPIO 12 (for user interaction)
*   Display module (screen)
*   Audio output devices (speakers)
*   Audio input device (mic)
*   Stable Wi-Fi connection (2.4G)
*   Serial control terminal (Baud rate 115200)

TUYA_T5AI Development Board is recommended to be developed using the T5AI-Board Development Kit.

## Configuration documentation

### Device credentials

Before uploading the code, you need to configure the following parameters:

```cpp
#define TUYA_DEVICE_UUID "uuidxxxxxxxxxxxxxxxx"
#define TUYA_DEVICE_AUTHKEY "keyxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
#define TUYA_PRODUCT_ID "alon7qgyjj8yus74"
```

You can create a product on the [Tuya Developer Platform](https://tuyaopen.ai/en/docs/cloud/tuya-cloud/creating-new-product) to obtain a PID or use the default PID.

For the method of obtaining authorization codes, refer to [Authorization Code Retrieval](https://tuyaopen.ai/en/docs/quick-start#tuyaopen-%E6%8E%88%E6%9D%83%E7%A0%81%E8%8E%B7%E5%8F%96). You must replace your own device UUID and AUTHKEY.

### Language selection

Set the language of the audio prompt.

```cpp
#define LANG_CODE_ZH 0  // 0: English, 1: Chinese
```

### Dialogue mode

```cpp
#define AI_AUDIO_WORK_MODE AI_AUDIO_WORK_ASR_WAKEUP_SINGLE_TALK // choice work mode：provide 4 modes to chat with AI Agent

#define AI_AUDIO_MODE_MANUAL_SINGLE_TALK     1
#define AI_AUDIO_WORK_VAD_FREE_TALK          2
#define AI_AUDIO_WORK_ASR_WAKEUP_SINGLE_TALK 3
#define AI_AUDIO_WORK_ASR_WAKEUP_FREE_TALK   4
```

### Hardware pins

```cpp
#define LED_PIN 1           // Status LED
#define BUTTON_PIN0 12      // Interaction button
```

### Data points

```cpp
#define DPID_VOLUME 3       // Volume control DP ID
```

## Usage method

### Initial settings

1.  **Upload code**: Modify the authorization code and burn the code onto the development board.
2.  Open the serial monitor: Set the波特率为115200.
3.  **Waiting for initialization**: The device will display system information.
4.  Device Network: Use [Tuya APP](https://tuyaopen.ai/en/docs/quick-start/device-network-configuration) for network.

### Interaction mode

#### Text chat (Serial)

Input messages to chat with AI in the serial monitor:

```
🧍‍♂️ User: How's the weather today?
AI: Today xxxxxxxxxx
================chat end=================
```

#### Voice interaction (button)

##### Wake word

*   **Hello, Tuya.**
*   **Hey Tuya**

Supports modifying macro definitions `AI_AUDIO_WORK_MODE` to switch between various [interaction modes](https://tuyaopen.ai/en/docs/applications/tuya.ai/ai-components/ai-audio-asr-impl)

#### LED indicator

*   **Power Off**：Power off
*   **Lighting**: Listening/Activity State

## Core functions

### Main components

*   **`setup()`**：Initialize all hardware, IoT connections, display, and AI components.
*   **`loop()`**：Handling continuous user input and state management
*   **`handleUserInput()`**：Handle text input from the serial monitor.
*   **`buttonCallback()`**：Handle the button press event for voice interaction.
*   **`tuyaIoTEventCallback()`**：Manage IoT events and network status
*   **`tuyaAIEventCallback()`**：Handle AI audio events (ASR, text, emotions)
*   **`tuyaAIStateCallback()`**：Update system state (standby, listening, uploading, playing)

### Button configuration

```cpp
ButtonConfig_t cfg = {
  .debounceTime = 50,              // Button debounce time (ms)
  .longPressTime = 2000,           // Long press threshold (ms)
  .longPressHoldTime = 500,        // Long press repeat interval (ms)
  .multiClickCount = 2,            // Maximum number of clicks for multi-click detection
  .multiClickInterval = 500        // Multi-click time window (ms)
};
```

### AI audio configuration

```cpp
AI_AUDIO_CONFIG_T ai_audio_cfg = {
  .work_mode = AI_AUDIO_WORK_MODE,
  .evt_inform_cb = tuyaAIEventCallback,
  .state_inform_cb = tuyaAIStateCallback,
};
```

## AI audio state

The system transitions between multiple states.

1.  **STANDBY**：Prepare to wake up or manually activate.
2.  **LISTEN**：Record user voice input
3.  **UPLOAD**：Send audio to the cloud for processing
4.  **AI_SPEAK**：Play AI response audio

## Display integration

If you have enabled `ENABLE_CHAT_DISPLAY`, the system will display:

`ENABLE_CHAT_DISPLAY` is enabled by default, and LVGL display is initialized in `app_display_init`. Users can create custom displays in `app_display.cpp` using the LVGL API.

The current display integration includes:

*   **User Message**: Transcription of ASR text from user voice
*   **AI message**：AI's text response (streaming or complete)
*   **Emotion**: AI emotions are represented by emoticons.
*   **State**: Current system state (listening, playing, standby)
*   **Network Status**: Wi-Fi Connection Quality
*   **Volume Level**: Current audio volume

## Data point processing

What is [DP point? What is it?](https://tuyaopen.ai/en/docs/applications)

### Volume control (DP ID 3)

Receive volume commands from the cloud:

```cpp
case DPID_VOLUME: {
  uint8_t volume = dp->value.dp_value;
  TuyaAI.setVolume(volume);
  // Send volume command to cloud
}
```

After initialization, the volume will automatically be uploaded to the cloud.

## Event handling

### IoT event handling: tuyaIoTEventCallback()

*   **TUYA_EVENT_RESET:** IOT device is reset.
*   **TUYA_EVENT_BIND_START:** Device pairing started
*   **TUYA_EVENT_BIND_TOKEN_ON:** Device pairing successful
*   **TUYA_EVENT_ACTIVATE_SUCCESSED:** Device activation successful
*   **TUYA_EVENT_MQTT_CONNECTED:** Cloud connection established
*   **TUYA_EVENT_MQTT_DISCONNECT:** Cloud connection disconnected
*   **TUYA_EVENT_DP_RECEIVE:** Received data point command
*   **TUYA_EVENT_DP_RECEIVE_CJSON:** Received JSON format data point
*   **TUYA_EVENT_DP_RECEIVE_OBJ:** Received object format data point
*   **TUYA_EVENT_DP_RECEIVE_RAW:** Received raw data
*   **TUYA_EVENT_UPGRADE_NOTIFY:** OTA Update Notification
*   **TUYA_EVENT_RESET_COMPLETE:** Device reset complete
*   **TUYA_EVENT_TIMESTAMP_SYNC:** Timestamp synchronization
*   **TUYA_EVENT_DPCACHE_NOTIFY:** Data point cache notification
*   **TUYA_EVENT_BINDED_NOTIFY:** Binding notification
*   **TUYA_EVENT_DIRECT_MQTT_CONNECTED:** Direct MQTT connection successful

### AI audio event handling: tuyaAIEventCallback()

*   **AI_AUDIO_EVT_NONE:** No events
*   **AI_AUDIO_EVT_HUMAN_ASR_TEXT:** Voice transcription text
*   **AI_AUDIO_EVT_AI_REPLIES_TEXT_START:** AI response text starts
*   **AI_AUDIO_EVT_AI_REPLIES_TEXT_DATA:** AI response text data
*   **AI_AUDIO_EVT_AI_REPLIES_TEXT_END:** AI response text ends
*   **AI_AUDIO_EVT_AI_REPLIES_TEXT_INTERUPT:** AI response text interrupted
*   **AI_AUDIO_EVT_AI_REPLIES_EMO:** AI emotions
*   **AI_AUDIO_EVT_ASR_WAKEUP:** ASR Wakeup

### AI audio state handling: tuyaAIStateCallback()

*   **AI_AUDIO_STATE_STANDBY:** Standby state
*   **AI_AUDIO_STATE_LISTEN:** Listening state
*   **AI_AUDIO_STATE_UPLOAD:** Upload state
*   **AI_AUDIO_STATE_AI_SPEAK:** Playback state

### Button event callback: buttonEventCallback()

*   **BUTTON_EVENT_PRESS_DOWN:** Press down on the button
*   **BUTTON_EVENT_PRESS_UP:** Press the button up
*   **BUTTON_EVENT_SINGLE_CLICK:** Single click
*   **BUTTON_EVENT_DOUBLE_CLICK:** Double click
*   **BUTTON_EVENT_MULTI_CLICK:** Multi-click
*   **BUTTON_EVENT_LONG_PRESS_START:** Long press start
*   **BUTTON_EVENT_LONG_PRESS_HOLD:** Long press hold

## Audio prompt:

System play various audio prompts:

*   **AI_AUDIO_ALERT_WAKEUP**：Wake-up confirmation sound
*   **AI_AUDIO_ALERT_NETWORK_CFG**：Network configuration mode
*   **AI_AUDIO_ALERT_NETWORK_CONNECTED**：Successfully connected to the cloud.
*   **AI_AUDIO_ALERT_NOT_ACTIVE**：Device has not been activated.

## Project Structure

```
your_chat_bot/
├── your_chat_bot.ino        # Main program file
├── app_config.h             # Application configuration
├── app_display.h            # Display interface header file
├── app_display.cpp          # Display implementation
├── app_system_info.h        # System information header file
├── app_system_info.cpp      # System information implementation
└── README.md                # This document
```

## Note-taking Tips

*   The device requires an active internet connection for AI processing.
*   Press the reset button 3 times during the startup to clear the network configuration.
*   Audio processing is performed in the cloud, requiring a stable network connection.
*   Each event will record the idle heap memory to monitor the system's health.

## Related documents

*   [TuyaOpen website](https://tuyaopen.ai)
*   [Github Code Repository](https://github.com/tuya/TuyaOpen)
*   [Arduino-TuyaOpen Repository](https://github.com/tuya/Arduino-TuyaOpen)
*   [Device Network Connection](https://tuyaopen.ai/en/docs/quick-start/device-network-configuration) (Requires downloading an APP)

## Contact us

If you have any good suggestions or feedback, please submit an issue on [GitHub](https://github.com/tuya/TuyaOpen/issues) !
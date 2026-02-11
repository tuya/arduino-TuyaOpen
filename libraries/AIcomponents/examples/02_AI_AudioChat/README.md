# AI Audio Chat

This example demonstrates how to implement a full-featured AI audio chat using the Arduino-TuyaOpen framework. Users can converse with the AI large language model via voice or serial text input, with support for multiple conversation mode switching, button interaction, and cloud DP volume control.

> This example only supports the TUYA_T5AI platform. It is recommended to use the [T5AI-Board Development Kit](https://tuyaopen.ai/en/docs/hardware-specific/t5-ai-board/overview-t5-ai-board) for development.

## Flashing Procedure

0. Make sure you have completed the development environment setup described in [Quick Start](Quick_start.md).

1. Connect the T5AI development board to your computer, open Arduino IDE, select the `TUYA_T5AI` board, and choose the correct upload port.

> Note: T5AI series development boards provide dual serial port communication. When connected to a computer, two serial port numbers will be detected. UART0 is used for firmware flashing — please select the correct upload port in Arduino IDE.

2. In Arduino IDE, click `File` -> `Examples` -> `AI components` -> `02_AI_AudioChat` to open the example code.

3. Replace the authorization code and product PID in the example file with your own credentials.
    - [What is an authorization code](https://tuyaopen.ai/en/docs/quick-start#tuyaopen-authorization-code)
    - [How to obtain an authorization code](https://tuyaopen.ai/en/docs/quick-start#obtaining-tuyaopen-authorization-code)

```cpp
// Device authorization code (replace with your own)
#define TUYA_DEVICE_UUID    "uuidxxxxxxxxxxxxxxxx"
#define TUYA_DEVICE_AUTHKEY "keyxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
// Product PID (replace with the PID created on the Tuya IoT platform)
#define TUYA_PRODUCT_ID     "xxxxxxxxxxxxxxxx"
```

4. Click the upload button in the top-left corner of Arduino IDE to flash the code. The following messages in the terminal indicate a successful flash.

```bash
[INFO]: Write flash success
[INFO]: CRC check success
[INFO]: Reboot done
[INFO]: Flash write success.
```

## Device Connection and Interaction

### Device Connection

After flashing the firmware, the development board needs to connect to the network and register with the cloud to enable remote communication. Once provisioned, three-way communication between `Phone` - `Tuya Cloud` - `Device` is established.

Device provisioning guide: [Device Network Configuration](https://tuyaopen.ai/en/docs/quick-start/device-network-configuration)

> **Quick Re-provisioning**: Press the reset button 3 times in quick succession, and the device will clear its saved network configuration and re-enter the provisioning state.

### Device Interaction

After successful provisioning via the phone APP, you can interact with the AI Agent in the following two ways.

#### Text Interaction

Open the Arduino IDE built-in Serial Monitor (baud rate 115200), type text in the input box and press Enter to send. You can then have a text conversation with the AI Agent via the serial port.

#### Voice Interaction

Voice interaction supports four conversation modes, with the default configuration set to `Wakeup` mode. Users can cycle through modes by **double-clicking the button**.

**Wake word**: 你好涂鸦 (Hey Tuya)

| Mode ID | Mode Name | Description |
| :---: | :---: | --- |
| 0 | Hold (HOLD) | User must press and hold the button to start voice input; releasing the button ends voice input |
| 1 | One-Shot (ONE_SHOT) | Device enters standby on power-up; a short press puts the device into listening state for a single voice conversation |
| 2 | Wakeup (WAKEUP) | User wakes the device with the wake word, then conducts a single conversation — similar to smart speaker interaction |
| 3 | Free (FREE) | After waking the device with the wake word, continuous conversation is enabled; if no conversation occurs within 30 seconds after wakeup, the device returns to standby |

## Example Code Explanation

### Hardware Configuration

```cpp
#define LED_PIN     9    // LED pin
#define BUTTON_PIN  29   // Button pin
#define DPID_VOLUME 3    // Volume DP ID
```

### AI Initialization

```cpp
// Configure AI core parameters
AIConfig_t aiConfig = {AI_CHAT_MODE_WAKEUP, 70, aiEventCallback, aiStateCallback, nullptr};
TuyaAI.begin(aiConfig);
// Initialize the audio subsystem
TuyaAI.Audio.begin();
```

Unlike the `01_AI_TextChat` example, this example registers both `aiEventCallback` and `aiStateCallback` callback functions to support the full voice interaction flow.

### AI Event Callback

```cpp
static void aiEventCallback(AI_USER_EVT_TYPE_E event, uint8_t *data, uint32_t len, void *arg);
```

This example handles the following AI events:

| Event | Description | Callback Data |
| --- | --- | --- |
| `AI_USER_EVT_ASR_OK` | ASR speech recognition successful | Recognized text content |
| `AI_USER_EVT_TEXT_STREAM_START` | LLM text response stream begins | First segment of text data |
| `AI_USER_EVT_TEXT_STREAM_DATA` | LLM text response stream data | Text data chunk |

> When the user converses via voice, the text recognized by cloud ASR is returned through the `AI_USER_EVT_ASR_OK` event, making it convenient for developers to display what the user said on the serial port or screen.

### AI State Callback

```cpp
static void aiStateCallback(AI_MODE_STATE_E state);
```

The AI device transitions between different states during operation. `aiStateCallback` receives state change notifications. The state list is as follows:

| State | Description |
| --- | --- |
| `AI_MODE_STATE_IDLE` | Idle state |
| `AI_MODE_STATE_INIT` | Initializing |
| `AI_MODE_STATE_LISTEN` | Listening (capturing voice) |
| `AI_MODE_STATE_UPLOAD` | Uploading (voice data uploading to cloud) |
| `AI_MODE_STATE_THINK` | Thinking (waiting for LLM response) |
| `AI_MODE_STATE_SPEAK` | Speaking (TTS is playing) |

### Button Interaction

The `Button` component is used to encapsulate button functionality. By default, it uses GPIO 29, triggered on low level, with an internal pull-up.

The following button events are supported:

| Event | Behavior |
| --- | --- |
| Press / Release | Delegated to `TuyaAI.modeKeyHandle()` for processing |
| Single Click (`BUTTON_EVENT_SINGLE_CLICK`) | Delegated to `TuyaAI.modeKeyHandle()` — triggers the conversation action for the current mode |
| Double Click (`BUTTON_EVENT_DOUBLE_CLICK`) | Stops audio playback → Interrupts current conversation → Switches to next conversation mode → Plays mode prompt tone |
| Long Press (`BUTTON_EVENT_LONG_PRESS_START`) | Delegated to `TuyaAI.modeKeyHandle()` (starts recording in Hold mode) |

Core logic for double-click mode switching:

```cpp
if (event == BUTTON_EVENT_DOUBLE_CLICK) {
    TuyaAI.Audio.stop();           // Stop current audio playback
    TuyaAI.interruptChat();        // Interrupt current conversation
    AI_CHAT_MODE_E nextMode = TuyaAI.nextChatMode();  // Get next mode
    TuyaAI.saveModeConfig(nextMode, volume);           // Save mode configuration
    // Play the prompt tone for the corresponding mode
    AI_AUDIO_ALERT_TYPE_E alert = (AI_AUDIO_ALERT_TYPE_E)(AI_AUDIO_ALERT_LONG_KEY_TALK + (int)nextMode);
    TuyaAI.Audio.playAlert(alert);
}
```

### DP Volume Control

This example implements cloud-based volume control via DP (DPID = 3):

```cpp
// Receive the volume value sent from the APP
case DPID_VOLUME:
    uint8_t volume = dp->value.dp_value;
    TuyaAI.setVolume(volume);
    break;
```

Upon the device's first MQTT connection, the current volume is automatically reported:

```cpp
case TUYA_EVENT_MQTT_CONNECTED:
    if (gFirstConnect) {
        gFirstConnect = false;
        aiAudioVolumUpload();  // Report current volume to the cloud
    }
    break;
```

## Related Documentation

- [TuyaOpen Official Website](https://tuyaopen.ai)
- [GitHub Repository](https://github.com/tuya/TuyaOpen)

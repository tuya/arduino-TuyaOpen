# Application Development - AI Chatbot

Arduino-TuyaOpen provides a complete AI chatbot application example. Users can leverage the `YourChatBot` sample code along with TUYA-T5AI series development boards to quickly build a smart chatbot that supports voice interaction, text conversation, MCP tool invocation, and more.

### Project File Structure

```
YourChatBot/
├── YourChatBot.ino       # Main program entry; includes setup/loop, AI event callback, IoT event callback
├── appButton.cpp/.h      # Button interaction module; handles single click, double click, long press, etc.
├── appDisplay.cpp/.h     # UI display module; supports built-in WeChat-style UI and custom LVGL UI
├── appMCP.cpp/.h         # MCP tool registration module; demonstrates photo capture, volume, mode switch, device info tools
├── appStatus.cpp/.h      # Status monitoring module; periodically updates WiFi signal and system heap memory info
└── appAudioRecord.cpp/.h # Audio recording module; supports saving MIC and TTS data to SD card
```

## Flashing Procedure

0. Make sure you have completed the development environment setup described in [Quick Start](Quick_start.md).

1. Connect the T5AI development board to your computer, open Arduino IDE, select the `TUYA_T5AI` board, and choose the correct upload port.

> Note: T5AI series development boards provide dual serial port communication. When connected to a computer, two serial port numbers will be detected. UART0 is used for firmware flashing — please select the correct upload port in Arduino IDE.

2. In Arduino IDE, click `File` -> `Examples` -> `AI components` -> `YourChatBot` to open the example code.

3. Replace the authorization code and product PID in the example file with your own credentials.
    - [What is an authorization code](https://tuyaopen.ai/en/docs/quick-start#tuyaopen-authorization-code)
    - [How to obtain an authorization code](https://tuyaopen.ai/en/docs/quick-start#obtaining-tuyaopen-authorization-code)

```cpp
// Device authorization code (replace with your own)
#define TUYA_DEVICE_UUID    "uuidxxxxxxxxxxxxxxxx"
#define TUYA_DEVICE_AUTHKEY "keyxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"

```

4. (Optional) To enable the audio recording feature (save MIC and TTS audio data to SD card), set `ENABLE_AUDIO_RECORDING` to `1`:

```cpp
#define ENABLE_AUDIO_RECORDING  1   // Default is 0 (off); set to 1 to enable audio recording
```

5. Click the upload button in the top-left corner of Arduino IDE to flash the code. The following messages in the terminal indicate a successful flash.

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

Open the Arduino IDE built-in Serial Monitor (baud rate 115200), type text in the input box and press Enter to send. You can then have a text conversation with the AI Agent via the serial port. The example code sends the text received from the serial port to the cloud-based large language model via `TuyaAI.sendText()`, and the AI's response is displayed simultaneously on both the Serial Monitor and the screen.

#### Voice Interaction

Voice interaction supports four conversation modes, with the default configuration set to `Wakeup` mode. Users can cycle through modes by **double-clicking the button**.

**Wake word**: 你好涂鸦 (Hey Tuya)

| Mode ID | Mode Name | Description |
| :---: | :---: | --- |
| 0 | Hold (HOLD) | User must press and hold the button to start voice input; releasing the button ends voice input |
| 1 | One-Shot (ONE_SHOT) | Device enters standby on power-up; a short press puts the device into listening state for a single voice conversation |
| 2 | Wakeup (WAKEUP) | User wakes the device with the wake word, then conducts a single conversation — similar to smart speaker interaction |
| 3 | Free (FREE) | After waking the device with the wake word, continuous conversation is enabled; if no conversation occurs within 30 seconds after wakeup, the device returns to standby |

## Application Example Overview

As a full-featured demonstration, developers can start with the more concise [AI examples](AI_API_Development.md) to familiarize themselves with the AI development workflow.

### AI Development

#### Initializing the AI Module

```cpp
// 1. Configure and initialize AI core
AIConfig_t aiConfig = {AI_CHAT_MODE_WAKEUP, 70, aiEventCallback, aiStateCallback, NULL};
TuyaAI.begin(aiConfig);

// 2. Initialize audio subsystem
TuyaAI.Audio.begin();

// 3. Initialize UI subsystem
appDisplayInit(UI_TYPE);
```

**Parameter description:**

| Parameter | Description |
| --- | --- |
| `AI_CHAT_MODE_WAKEUP` | Initial conversation mode, options: `AI_CHAT_MODE_HOLD`(0) / `AI_CHAT_MODE_ONE_SHOT`(1) / `AI_CHAT_MODE_WAKEUP`(2) / `AI_CHAT_MODE_FREE`(3) |
| `70` | Initial volume (range 0–100) |
| `aiEventCallback` | AI event callback function for receiving all AI event data including ASR, TTS, text stream, etc. |
| `aiStateCallback` | AI state callback function for receiving device state change notifications |
| `NULL` | User-defined data pointer, typically set to `NULL` |

`TuyaAI.Audio.begin()` initializes the device's audio system, enabling the microphone, speaker, and audio codec functionality.

UI initialization is determined by the `UI_TYPE` macro in `appDisplay.h`:

```c
#define UI_TYPE             BOT_UI_WECHAT   // Modify in appDisplay.h
```

- `BOT_UI_WECHAT`: Built-in WeChat-style UI, ready to use out of the box
- `BOT_UI_USER`: Only initializes LVGL; users design the UI themselves via LVGL APIs

#### AI Event Callback

```cpp
static void aiEventCallback(AI_USER_EVT_TYPE_E event, uint8_t *data, uint32_t len, void *arg);
```

During AI operation, a rich set of events and data is generated. All AI events are dispatched and handled uniformly through `aiEventCallback`. The main event types are as follows:

| Event | Description | Callback Data |
| --- | --- | --- |
| `AI_USER_EVT_ASR_OK` | ASR speech recognition successful | Recognized text content |
| `AI_USER_EVT_ASR_EMPTY` | ASR result is empty (user did not speak) | None |
| `AI_USER_EVT_ASR_ERROR` | ASR recognition failed | None |
| `AI_USER_EVT_MIC_DATA` | Microphone raw PCM audio data | PCM audio byte stream |
| `AI_USER_EVT_TTS_PRE` | TTS is about to start playing | None |
| `AI_USER_EVT_TTS_START` | TTS starts playing | None |
| `AI_USER_EVT_TTS_DATA` | TTS audio data chunk (MP3 format) | MP3 audio byte stream |
| `AI_USER_EVT_TTS_STOP` | TTS playback completed normally | None |
| `AI_USER_EVT_TTS_ABORT` | TTS was interrupted | None |
| `AI_USER_EVT_TEXT_STREAM_START` | LLM text response stream begins | First segment of text data |
| `AI_USER_EVT_TEXT_STREAM_DATA` | LLM text response stream data | Text data chunk |
| `AI_USER_EVT_TEXT_STREAM_STOP` | LLM text response stream ends | None |
| `AI_USER_EVT_SKILL` | Skill data (e.g., music playback) | cJSON* skill data |
| `AI_USER_EVT_EMOTION` | Emotion parsed from text tags | `AI_AGENT_EMO_T*` emotion struct |
| `AI_USER_EVT_LLM_EMOTION` | User emotion inferred by LLM | `AI_AGENT_EMO_T*` emotion struct |
| `AI_USER_EVT_SERVER_VAD` | Server-side voice activity detection ended | None |
| `AI_USER_EVT_MODE_SWITCH` | Conversation mode switched | New mode number (int) |
| `AI_USER_EVT_CHAT_BREAK` | Conversation was interrupted | None |

#### AI State Callback

```cpp
static void aiStateCallback(AI_MODE_STATE_E state);
```

The AI device transitions between different states during operation. `aiStateCallback` receives state change notifications, and users can update the UI or execute business logic accordingly. The state list is as follows:

| State | Description |
| --- | --- |
| `AI_MODE_STATE_IDLE` | Idle state |
| `AI_MODE_STATE_INIT` | Initializing |
| `AI_MODE_STATE_LISTEN` | Listening (capturing voice) |
| `AI_MODE_STATE_UPLOAD` | Uploading (voice data uploading to cloud) |
| `AI_MODE_STATE_THINK` | Thinking (waiting for LLM response) |
| `AI_MODE_STATE_SPEAK` | Speaking (TTS is playing) |

### IoT Development

#### IoT Initialization

```cpp
// Press the reset button 3 times in quick succession to trigger device re-provisioning
TuyaIoT.resetNetcfg();
// Set IoT event callback function
TuyaIoT.setEventCallback(tuyaIoTEventCallback);
// Set device authorization code
TuyaIoT.setLicense(TUYA_DEVICE_UUID, TUYA_DEVICE_AUTHKEY);
// Start IoT service with product PID and firmware version
TuyaIoT.begin(TUYA_PRODUCT_ID, PROJECT_VERSION);
```

`TuyaIoT.resetNetcfg()` enables the "quick reset button re-provisioning" feature. When the user presses the reset button 3 times in quick succession, the device clears its saved network configuration and re-enters the provisioning state.

#### IoT Event Handling

```cpp
static void tuyaIoTEventCallback(tuya_event_msg_t *event);
```

All IoT platform events are handled uniformly in `tuyaIoTEventCallback`. The main events include:

| Event | Description |
| --- | --- |
| `TUYA_EVENT_BIND_START` | Device starts network provisioning; plays provisioning prompt tone |
| `TUYA_EVENT_MQTT_CONNECTED` | MQTT connection successful; MCP tools can be initialized and device status reported at this point |
| `TUYA_EVENT_MQTT_DISCONNECT` | MQTT connection lost |
| `TUYA_EVENT_TIMESTAMP_SYNC` | Cloud time synchronization |
| `TUYA_EVENT_RESET` | Device has been reset |
| `TUYA_EVENT_UPGRADE_NOTIFY` | OTA upgrade notification received |
| `TUYA_EVENT_DP_RECEIVE_OBJ` | Object-type DP data received (e.g., volume control) |
| `TUYA_EVENT_DP_RECEIVE_RAW` | Raw-type DP data received |

#### DP Data Interaction

This example demonstrates cloud-based volume control via DP (Data Point):

```cpp
#define DPID_VOLUME  3  // Volume DP ID

// Handle volume setting in DP receive callback
case DPID_VOLUME:
    uint8_t volume = dp->value.dp_value;
    TuyaAI.setVolume(volume);
    break;
```

Users can also leverage the Tuya Cloud for IoT development to control `powered by Tuya` devices. For more DP operations, refer to the `00_IoT_SimpleExample` example.

### Audio Development

All audio-related data is obtained through the `aiEventCallback` callback function, distinguished by `AI_USER_EVT_TYPE_E` event types:

| Event | Data Format | Description |
| --- | --- | --- |
| `AI_USER_EVT_ASR_OK` | UTF-8 text | Cloud ASR speech recognition result |
| `AI_USER_EVT_MIC_DATA` | PCM (16kHz, 16-bit, mono) | Raw microphone recording data |
| `AI_USER_EVT_TTS_DATA` | MP3 data chunks | TTS audio data stream from the LLM |
| `AI_USER_EVT_TTS_PRE/START/STOP` | None | TTS playback lifecycle events |
| `AI_USER_EVT_TEXT_STREAM_START/DATA/STOP` | UTF-8 text | Text data stream from the LLM |
| `AI_USER_EVT_SERVER_VAD` | None | Server-side voice activity detection (detects user stops speaking) |

#### Audio Recording to SD Card

The `appAudioRecord.cpp/.h` files demonstrate how to save MIC and TTS audio data to an SD card. Set the `ENABLE_AUDIO_RECORDING` macro to `1` before use.

- **MIC recording**: Saved in `.pcm` format with an in-memory buffer (default 16KB) to reduce SD card write frequency. Recording starts automatically when entering `AI_MODE_STATE_LISTEN` and stops when leaving the listening state.
- **TTS recording**: Saved in `.mp3` format; starts at `AI_USER_EVT_TTS_START` and ends at `AI_USER_EVT_TTS_STOP` or `AI_USER_EVT_TTS_ABORT`.
- Recording files are saved in the `/ai_recordings` directory on the SD card, with auto-incrementing sequential naming (e.g., `mic_001.pcm`, `tts_001.mp3`).

### Button Interaction

The `appButton.cpp/.h` files use the `Button` component to provide unified button functionality. By default, it uses GPIO 12, triggered on low level, with an internal pull-up.

The following button events are supported:

| Event | Behavior |
| --- | --- |
| Single Click (`BUTTON_EVENT_SINGLE_CLICK`) | Delegated to `TuyaAI.modeKeyHandle()` — triggers the conversation action for the current mode |
| Double Click (`BUTTON_EVENT_DOUBLE_CLICK`) | Stops audio playback → Interrupts current conversation → Switches to next conversation mode → Plays mode prompt tone |
| Long Press (`BUTTON_EVENT_LONG_PRESS_START`) | Delegated to `TuyaAI.modeKeyHandle()` (starts recording in Hold mode) |
| Press / Release | Delegated to `TuyaAI.modeKeyHandle()` for processing |

Core logic for double-click mode switching:

```cpp
if (event == BUTTON_EVENT_DOUBLE_CLICK) {
    TuyaAI.Audio.stop();           // Stop current audio playback
    TuyaAI.interruptChat();        // Interrupt current conversation
    AI_CHAT_MODE_E nextMode = TuyaAI.nextChatMode();  // Get next mode
    TuyaAI.saveModeConfig(nextMode, volume);           // Save mode configuration
    // Play the prompt tone for the corresponding mode
}
```

> For more usage of the Button component, refer to the examples under the `Peripheral/Button` directory.

### UI Development

The `appDisplay.cpp/.h` files provide unified encapsulation of the device's UI functionality. Switch between UI schemes by modifying the `UI_TYPE` macro in `appDisplay.h`.

#### BOT_UI_WECHAT

A built-in WeChat-style UI page, ready to use out of the box. Users can obtain fonts (`getTextFont`), icons (`getIconFont` / `getWifiIcon`), emojis, and other UI resources via the `TuyaAI.UI` class interfaces, and control the display content of built-in UI components.

#### BOT_UI_USER

This parameter only starts the LVGL graphics engine, allowing users to design the UI entirely on their own. The `_createUI()` function in `appDisplay.cpp` provides a minimalist dark-themed UI reference implementation with the following layout:

```
┌──────────────────────────┐
│ [Chat Mode]      [WiFi]  │ ← Top status bar (30px)
├──────────────────────────┤
│       [Device Status]    │ ← Status indicator area (36px)
├──────────────────────────┤
│ ┌────────────────────┐   │
│ │ User message...    │◀──│ ← User bubble (right-aligned, green)
│ └────────────────────┘   │
│   ┌────────────────────┐ │
│ ──▶ AI response...    │ │ ← AI bubble (left-aligned, gray)
│   └────────────────────┘ │
└──────────────────────────┘
```

The example also encapsulates the following display interfaces for convenient UI updates in callbacks:

| Interface | Description |
| --- | --- |
| `appDisplaySetUserText()` | Display user text |
| `appDisplaySetAIText()` | Display AI response text |
| `appDisplayStreamStart/Append/End()` | Stream-display AI response |
| `appDisplaySetStatus()` | Update device status text |
| `appDisplaySetMode()` | Update conversation mode display |
| `appDisplaySetWifi()` | Update WiFi status icon |

#### Display Component

Users can use this component to directly drive the screen on the TUYA-T5AI-BOARD development board. The component provides basic screen rendering capabilities and **camera data direct display** functionality.

> For more usage of the Display component, refer to the examples under the `Display` / `Camera` directories.

### AI Advanced Features

#### MCP

[What is MCP](https://tuyaopen.ai/en/docs/cloud/tuya-cloud/ai-agent/mcp-management): MCP (Model Context Protocol) is a universal protocol interface that enables AI Agent to invoke device-side local tools.

The `appMCP.cpp/.h` files demonstrate how to register local MCP tools for AI Agent invocation.

> **Note**: MCP tools must be registered after the MQTT connection is established. The example subscribes to the `EVENT_MQTT_CONNECTED` event to ensure correct registration timing.

**Two registration methods:**

1. **Simple registration** (parameterless tool): Use the `TUYA_MCP_TOOL_ADD_SIMPLE` macro

```cpp
// Register a parameterless tool
TUYA_MCP_TOOL_ADD_SIMPLE(
    "device_info_get",          // Tool name
    "Get device information.",  // Tool description (for AI to understand purpose)
    onGetDeviceInfo,            // Callback function
    nullptr                     // User data
);
```

2. **Registration with parameters**: Use the `TUYA_MCP_TOOL_REGISTER` macro + property definitions

```cpp
// Define tool parameters
TuyaMCPPropDef volumeProps[] = {
    TuyaMCPPropIntRange("volume", "The volume level (0-100).", 0, 100)
};
// Register tool with parameters
TUYA_MCP_TOOL_REGISTER(
    "device_audio_volume_set",  // Tool name
    "Sets the device volume.",  // Tool description
    onSetVolume,                // Callback function
    nullptr,                    // User data
    volumeProps, 1              // Parameter definitions and count
);
```

**Parameter type macros:**

| Macro | Description |
| --- | --- |
| `TuyaMCPPropStr(name, desc)` | String type parameter |
| `TuyaMCPPropIntRange(name, desc, min, max)` | Integer type parameter (with range constraint) |
| `TuyaMCPPropIntDefRange(name, desc, def, min, max)` | Integer type parameter (with default value and range) |

**Common methods in callback functions:**

| Method | Description |
| --- | --- |
| `TuyaAI.MCP.getPropertyInt(props, name, def)` | Get integer parameter value |
| `TuyaAI.MCP.setReturnBool(retVal, value)` | Return a boolean value |
| `TuyaAI.MCP.setReturnJson(retVal, json)` | Return a JSON object |
| `TuyaAI.MCP.setReturnImage(retVal, mime, data, len)` | Return image data (e.g., photo capture result) |

This example provides four MCP tools:

| Tool Name | Trigger Command | Function |
| --- | --- | --- |
| `device_info_get` | "Get device info" | Returns device model, serial number, firmware version, and other JSON information |
| `device_camera_take_photo` | "Take a photo" | Captures a photo with the camera and returns the JPEG image to the AI for content recognition |
| `device_audio_volume_set` | "Set the volume to 50" | Sets device volume (0–100) |
| `device_audio_mode_set` | "Switch to free conversation mode" | Switches conversation mode (0=Hold, 1=One-Shot, 2=Wakeup, 3=Free) |

#### Skill

Skill data is parsed from the `AI_USER_EVT_SKILL` event in `aiEventCallback`. Use the `TuyaAI.Skill.parse()` method for parsing, which supports multiple skill types:

```cpp
case AI_USER_EVT_SKILL:
    SkillData_t skill;
    if (OPRT_OK == TuyaAI.Skill.parse(data, skill)) {
        switch (skill.type) {
            case SKILL_TYPE_MUSIC:          // Music playback
            case SKILL_TYPE_STORY:          // Story playback
                // Get music/story URL, name, and other information
                // skill.music->src_array[0].url
                TuyaAI.Skill.dumpMusic(skill);   // Print debug info
                TuyaAI.Skill.freeMusic(skill);   // Release resources (must be called)
                break;
            case SKILL_TYPE_PLAY_CONTROL:   // Playback control (previous/next/pause, etc.)
                // skill.playControl.action
                break;
            case SKILL_TYPE_EMOTION:        // Emotion skill
                break;
        }
    }
    break;
```

#### Emotion

Emotion recognition during AI chat comes from two sources:

- `AI_USER_EVT_EMOTION`: Emotion parsed from text tags
- `AI_USER_EVT_LLM_EMOTION`: User emotion inferred by the large language model

Both return data via the `AI_AGENT_EMO_T` structure, containing the **emotion name** (`name`) and the **corresponding emoji** (`emoji`, in Unicode encoding). You can use `TuyaAI.Skill.unicodeToUtf8()` to convert the emoji's Unicode encoding to a UTF-8 string for display.

```cpp
case AI_USER_EVT_LLM_EMOTION:
    AI_AGENT_EMO_T *emo = (AI_AGENT_EMO_T *)data;
    Serial.print("Emotion: "); Serial.println(emo->name);
    if (emo->emoji) {
        char utf8[8];
        TuyaAI.Skill.unicodeToUtf8(emo->emoji, utf8, sizeof(utf8));
        Serial.print("Emoji: "); Serial.println(utf8);
    }
    break;
```

### Status Monitoring

The `appStatus.cpp/.h` files implement periodic monitoring of device runtime status, including:

- **WiFi signal status**: Checks WiFi connection status every 1 second; automatically updates the WiFi icon on the screen when the status changes
- **Heap memory monitoring**: Outputs the free heap memory size every 5 seconds for development debugging
- **Conversation mode display**: Displays the current conversation mode name on the screen in real time

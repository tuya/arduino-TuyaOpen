# YourChatBot - AI Chatbot Example

## Overview
This example shows how to build a complete AI chatbot on the Tuya T5AI platform using the TuyaAI library. It provides an extensible framework including UI rendering, button interactions, MCP tool extensions, and optional audio recording.

## Wake Words
- **你好涂鸦**
- **Hey Tuya**

## Features
- **Multiple chat modes**: 4 modes (button / wake word / free talk, etc.)
- **Customizable UI**: use a built-in UI template or your own LVGL UI
- **MCP tool extensions**: register custom MCP tools (e.g. take photo, get device info)
- **Event-driven architecture**: rich AI event callbacks (ASR, TTS, emotion, skill, etc.)
- **Audio recording**: optional MIC/TTS recording to SD card
- **Modular design**: independent modules for button, display, status monitoring

## Hardware Requirements
- Tuya `TUYA_T5AI` development board
- LED on GPIO 1 (status indicator)
- Button on GPIO 12 (mode switch / voice trigger)
- Speaker on GPIO 28
- Stable Wi-Fi connection
- Serial console (baud rate 115200)

Recommended: [T5AI-Board Development Kit](https://tuyaopen.ai/zh/docs/hardware-specific/t5-ai-board/overview-t5-ai-board)

## Project Structure

```
YourChatBot/
├── YourChatBot.ino      # Main entry
├── appDisplay.cpp/h     # UI module (supports custom LVGL)
├── appButton.cpp/h      # Button interaction module
├── appMCP.cpp/h         # MCP tool registration module
├── appStatus.cpp/h      # Status monitor (Wi-Fi, memory, etc.)
└── appAudioRecord.cpp/h # Audio recording module (optional)
```

## Configuration
Before uploading, configure the following parameters:

```cpp
// Device authorization (MUST be replaced with your own)
#define TUYA_DEVICE_UUID    "uuidxxxxxxxxxxxxxxxx"
#define TUYA_DEVICE_AUTHKEY "keyxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
#define TUYA_PRODUCT_ID     "9inb01mvjqh5zhhr"

// Pin configuration
#define LED_PIN             1
#define BUTTON_PIN          12
#define SPK_PIN             28

// Feature switch
#define ENABLE_AUDIO_RECORDING  0   // Set to 1 to enable audio recording
```

### How to Get Credentials
- **PID (Product ID)**: create a product on the [Tuya Developer Platform](https://tuyaopen.ai/zh/docs/cloud/tuya-cloud/creating-new-product)
- **UUID/AUTHKEY**: see [Authorization Code Acquisition Guide](https://tuyaopen.ai/zh/docs/quick-start#tuyaopen-%E6%8E%88%E6%9D%83%E7%A0%81%E8%8E%B7%E5%8F%96)

### Chat Mode

```cpp
AIConfig_t aiConfig = {
	.chatMode = AI_MODE_WAKEUP,  // Default: wake word mode
	.volume = 70,
	.eventCb = aiEventCallback,
	.stateCb = aiStateCallback,
	.userArg = NULL
};
```

Available modes:
| Mode | Description |
|------|-------------|
| `AI_MODE_HOLD` | Hold the button to talk |
| `AI_MODE_ONESHOT` | Click once for a single turn |
| `AI_MODE_WAKEUP` | Wake word + single turn |
| `AI_MODE_FREE` | Free talk |

### UI Type
Configure in `appDisplay.h`:

```cpp
#define UI_TYPE  BOT_UI_WECHAT  // Options: BOT_UI_USER, BOT_UI_WECHAT, BOT_UI_CHATBOT
```

## How to Use
1. **Configure credentials**: replace UUID, AUTHKEY, and PID
2. **Upload**: upload the sketch to the T5AI board
3. **Open Serial**: 115200 baud
4. **Provision & bind**: scan QR code with the Tuya Smart app to bind
5. **Start chatting**:
   - Voice: say "ni hao tuya" or "Hey Tuya"
   - Text: type in Serial Monitor and press Enter

### Button Actions
| Action | Function |
|--------|----------|
| Single click | Trigger voice input (depends on mode) |
| Double click | Switch chat mode |
| Long press | Hold-to-talk (HOLD mode) |

## Core Components

### TuyaAI Architecture

```cpp
TuyaAI              // Main controller
├── TuyaAI.UI       // UI management
├── TuyaAI.Audio    // Audio input/output
├── TuyaAI.MCP      // MCP tool protocol
└── TuyaAI.Skill    // Skill payload parsing
```

### Event Callback

```cpp
void aiEventCallback(AIEvent_t event, uint8_t *data, uint32_t len, void *arg) {
	switch (event) {
		case AI_USER_EVT_ASR_OK:      // ASR success
		case AI_USER_EVT_TTS_START:   // TTS starts
		case AI_USER_EVT_EMOTION:     // Emotion
		case AI_USER_EVT_SKILL:       // Skill event (music, story, etc.)
		// ... more events
	}
}
```

### MCP Tool Example

Register custom tools in `appMCP.cpp`:

```cpp
// Get device info tool
TuyaAI.MCP.registerTool(
	"get_device_info",
	"Get device information",
	onGetDeviceInfo, NULL,
	{}  // No args
);

// Take photo tool
TuyaAI.MCP.registerTool(
	"take_photo",
	"Take a photo with the camera",
	onTakePhoto, NULL,
	{TuyaMCPPropStrDef("quality", "Image quality", "medium")}
);
```

## Troubleshooting

| Issue | Solution |
|------|----------|
| Device cannot connect | Check Wi-Fi credentials and ensure the network can reach Tuya Cloud |
| No AI response | Verify the device is bound and MQTT is connected |
| No serial output | Ensure baud rate is 115200 |
| Wake word not working | Check microphone connection and reduce ambient noise |
| UI not showing | Check `UI_TYPE` config and ensure LVGL is enabled |

## Notes
- Press reset quickly 3 times to clear network configuration
- GPIO 1 LED indicates device status
- Audio recording requires SD card support
- MCP tools can be registered only after MQTT is connected

## Related Documents
- [TuyaOpen Official Website](https://tuyaopen.ai)
- [GitHub Repository](https://github.com/tuya/TuyaOpen)
- [Arduino-TuyaOpen Repository](https://github.com/tuya/Arduino-TuyaOpen)
- [Device Network Configuration](https://tuyaopen.ai/en/docs/quick-start/device-network-configuration) (requires app download)

## Contact Us
If you have questions or suggestions, please open an issue on GitHub:
- https://github.com/tuya/TuyaOpen/issues

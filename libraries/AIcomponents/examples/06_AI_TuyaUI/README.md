# AI Custom UI Display

This example demonstrates how to use the **TuyaAI.UI** module in the Arduino-TuyaOpen framework together with the **LVGL** graphics library to create a custom AI chat interface on the T5AI development board's screen. The user's spoken text and the AI's responses are displayed on the screen in real time, along with device status and WiFi connection information.

> This example only supports the TUYA_T5AI platform with a display. It is recommended to use the [T5AI-Board Development Kit](https://tuyaopen.ai/en/docs/hardware-specific/t5-ai-board/overview-t5-ai-board) for development.

### Project File Structure

```
06_AI_TuyaUI/
├── 06_AI_TuyaUI.ino       # Main program entry; drives UI updates in AI event/state callbacks
└── ui_display.cpp/.h      # UI display module; encapsulates LVGL component creation and text updates
```

## Flashing Procedure

0. Make sure you have completed the development environment setup described in [Quick Start](Quick_start.md).

1. Connect the T5AI development board to your computer, open Arduino IDE, select the `TUYA_T5AI` board, and choose the correct upload port.

> Note: T5AI series development boards provide dual serial port communication. When connected to a computer, two serial port numbers will be detected. UART0 is used for firmware flashing — please select the correct upload port in Arduino IDE.

2. In Arduino IDE, click `File` -> `Examples` -> `AI components` -> `06_AI_TuyaUI` to open the example code.

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

After flashing the firmware, the development board needs to connect to the network and register with the cloud.

Device provisioning guide: [Device Network Configuration](https://tuyaopen.ai/en/docs/quick-start/device-network-configuration)

> **Quick Re-provisioning**: Press the reset button 3 times in quick succession, and the device will clear its saved network configuration and re-enter the provisioning state.

### Device Interaction

After successful provisioning, you can converse with the AI via voice or serial text input. The screen displays the following in real time:
- **User text** (blue): Speech recognition results or serial input content
- **AI response** (white): LLM text response (streaming updates)
- **Device status** (green): Current AI working state (Ready / Listening / Thinking, etc.)
- **WiFi icon**: Network connection status

## Example Code Explanation

### UI Initialization

This example uses `BOT_UI_USER` mode, which only initializes the LVGL graphics engine — the UI layout is entirely designed by the user:

```cpp
// Initialize the Display component
gDisplay.begin();
gDisplay.setBrightness(80);

// Initialize the AI UI subsystem (user-defined mode)
TuyaAI.UI.begin(BOT_UI_USER);

// Create custom LVGL UI
uiInit();
```

**UI mode description:**

| Mode | Description |
| --- | --- |
| `BOT_UI_WECHAT` | Built-in WeChat-style UI, ready to use out of the box |
| `BOT_UI_USER` | Only initializes the LVGL engine; users design the UI themselves |

### UI Layout

The `uiInit()` function in `ui_display.cpp` creates a minimalist dark-themed chat interface:

```
┌──────────────────────────┐
│ [Ready]          [WiFi]  │ ← Top status bar
├──────────────────────────┤
│ User: (user speech text) │ ← User text (blue #4fc3f7)
│                          │
│ AI: (AI response text)   │ ← AI text (white #ffffff)
│                          │
└──────────────────────────┘
     Background: #1a1a2e
```

The UI components use `TuyaAI.UI.getTextFont()` to obtain the system-provided CJK/English font, ensuring proper Chinese character rendering.

### AI Event Callback Drives UI Updates

In `aiEventCallback`, AI-generated data is updated to the screen in real time:

| Event | UI Update Logic |
| --- | --- |
| `AI_USER_EVT_ASR_OK` | Call `uiSetUser(text)` to display user speech text |
| `AI_USER_EVT_TEXT_STREAM_START` | Call `uiResetAIBuffer()` to reset the AI text buffer and start appending |
| `AI_USER_EVT_TEXT_STREAM_DATA` | Call `uiAppendAIBuffer()` to append text and update the AI label in real time |
| `AI_USER_EVT_TEXT_STREAM_STOP` | Call `uiFlushAIBuffer()` to finalize the current response |

```cpp
case AI_USER_EVT_ASR_OK:
    if (data && len > 0) {
        char text[256];
        int n = (len < sizeof(text) - 1) ? len : sizeof(text) - 1;
        memcpy(text, data, n);
        text[n] = '\0';
        uiSetUser(text);   // Update user text
    }
    break;

case AI_USER_EVT_TEXT_STREAM_START:
    uiResetAIBuffer();              // Reset AI buffer
    uiAppendAIBuffer(data, len);    // Append first text segment
    break;

case AI_USER_EVT_TEXT_STREAM_DATA:
    uiAppendAIBuffer(data, len);    // Append text data
    break;

case AI_USER_EVT_TEXT_STREAM_STOP:
    uiFlushAIBuffer();              // Finalize response
    break;
```

> The AI's text response is displayed using a streaming buffer mechanism: each time a new text chunk is received, it is appended to the buffer and the LVGL label is refreshed, allowing the user to see the AI response appear character by character.

### AI State Callback Drives Status Display

In `aiStateCallback`, the status text on the screen is updated:

```cpp
static void aiStateCallback(AI_MODE_STATE_E state) {
    const char *s = "Ready";
    switch (state) {
        case AI_MODE_STATE_LISTEN: s = "Listening..."; break;
        case AI_MODE_STATE_UPLOAD: s = "Processing..."; break;
        case AI_MODE_STATE_THINK:  s = "Thinking..."; break;
        case AI_MODE_STATE_SPEAK:  s = "Speaking..."; break;
        default: break;
    }
    uiSetStatus(s);
}
```

### UI Display Interface

The following interfaces are encapsulated in `ui_display.h`:

| Interface | Description |
| --- | --- |
| `uiInit()` | Initialize UI and create LVGL components |
| `uiSetUser(text)` | Display user text |
| `uiSetAI(text)` | Display AI response text |
| `uiSetStatus(text)` | Update device status text |
| `uiSetWifi(connected)` | Update WiFi status icon color |
| `uiResetAIBuffer()` | Reset AI text buffer |
| `uiAppendAIBuffer(data, len)` | Append AI text to the buffer and refresh display |
| `uiFlushAIBuffer()` | Finalize the current AI response |

### TuyaAI.UI Resource Interface

`TuyaAI.UI` provides the following font and icon resources for convenient custom UI development:

| Interface | Description |
| --- | --- |
| `TuyaAI.UI.getTextFont()` | Get CJK/English text font |
| `TuyaAI.UI.getIconFont()` | Get FontAwesome icon font |
| `TuyaAI.UI.getEmoFont()` | Get emoji font |
| `TuyaAI.UI.getEmoList()` | Get emoji list |
| `TuyaAI.UI.getWifiIcon(status)` | Get WiFi status icon character |
| `TuyaAI.UI.displayText(text, isUser)` | Display text via built-in interface |
| `TuyaAI.UI.displayWifiStatus(status)` | Display WiFi status via built-in interface |
| `TuyaAI.UI.displayChatMode(mode)` | Display conversation mode via built-in interface |

## Related Documentation

- [TuyaOpen Official Website](https://tuyaopen.ai)
- [GitHub Repository](https://github.com/tuya/TuyaOpen)

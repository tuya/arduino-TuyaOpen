# AI Text Chat

This example demonstrates how to implement a minimal AI text chat feature using the Arduino-TuyaOpen framework. Users can send text via the serial port to converse with the cloud-based AI large language model, and the AI's responses are output in real-time to the Serial Monitor as streaming text.

> This is the simplest entry-level example among all AI examples, containing only the core logic for text interaction — no buttons, UI, or audio interaction required.

## Flashing Procedure

0. Make sure you have completed the development environment setup described in [Quick Start](Quick_start.md).

1. Connect the T5AI development board to your computer, open Arduino IDE, select the `TUYA_T5AI` board, and choose the correct upload port.

> Note: T5AI series development boards provide dual serial port communication. When connected to a computer, two serial port numbers will be detected. UART0 is used for firmware flashing — please select the correct upload port in Arduino IDE.

2. In Arduino IDE, click `File` -> `Examples` -> `AI components` -> `01_AI_TextChat` to open the example code.

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

### Text Interaction

Open the Arduino IDE built-in Serial Monitor (baud rate 115200), type text in the input box and press Enter to send. You can then have a text conversation with the AI large language model via the serial port.

Interaction example:

```
[User]: How's the weather today?
[AI]: Today's weather is sunny with mild temperatures...

[User]: Tell me a joke
[AI]: Why do programmers prefer dark mode? Because bugs are easier to hide in the dark!
```

## Example Code Explanation

### AI Initialization

```cpp
// Configure AI core parameters
AIConfig_t cfg = {AI_CHAT_MODE_WAKEUP, 70, aiEventCallback, nullptr, nullptr};
TuyaAI.begin(cfg);
// Initialize the audio subsystem (required for TTS playback)
TuyaAI.Audio.begin();
```

**Parameter description:**

| Parameter | Description |
| --- | --- |
| `AI_CHAT_MODE_WAKEUP` | Initial conversation mode (this example uses text only; this parameter does not affect text interaction) |
| `70` | Initial volume (range 0–100) |
| `aiEventCallback` | AI event callback function for receiving AI response text streams |
| `nullptr` | AI state callback function (not used in this example) |
| `nullptr` | User-defined data pointer |

> In this example, the state callback is set to `nullptr` because a pure text chat scenario does not require monitoring device state changes. For more complex interactions (e.g., voice or UI), a state callback should be registered.

### Sending Text

Use `TuyaAI.sendText()` to send text received from the serial port to the cloud-based large language model:

```cpp
static void handleUserInput(void) {
    static int i = 0;
    static uint8_t _recv_buf[256];
    while (Serial.available()) {
        char c = Serial.read();
        _recv_buf[i++] = c;
        if (c == '\n' || c == '\r') {
            TuyaAI.sendText(_recv_buf, i);
            i = 0;
            memset(_recv_buf, 0, sizeof(_recv_buf));
        }
    }
}
```

### AI Event Callback

This example handles only the three **text stream (TEXT_STREAM)** events:

```cpp
static void aiEventCallback(AI_USER_EVT_TYPE_E event, uint8_t *data, uint32_t len, void *arg)
{
    switch (event) {
        case AI_USER_EVT_TEXT_STREAM_START:
            Serial.print("[AI]: ");
            if (data && len > 0) Serial.write(data, len);
            break;
        case AI_USER_EVT_TEXT_STREAM_DATA:
            if (data && len > 0) Serial.write(data, len);
            break;
        case AI_USER_EVT_TEXT_STREAM_STOP:
            Serial.println("\n");
            break;
        default:
            break;
    }
}
```

| Event | Description | Callback Data |
| --- | --- | --- |
| `AI_USER_EVT_TEXT_STREAM_START` | LLM text response stream begins | First segment of text data |
| `AI_USER_EVT_TEXT_STREAM_DATA` | LLM text response stream data | Text data chunk |
| `AI_USER_EVT_TEXT_STREAM_STOP` | LLM text response stream ends | None |

> The AI's text response uses streaming transmission. The `START` event may carry the first segment of text data, followed by multiple `DATA` events that progressively return the complete response, and finally a `STOP` event signals the end of the reply.

### IoT Event Handling

The IoT event handling in this example is minimal, covering only basic provisioning and connection events:

| Event | Description |
| --- | --- |
| `TUYA_EVENT_BIND_START` | Device starts network provisioning and plays a provisioning prompt tone |
| `TUYA_EVENT_MQTT_CONNECTED` | MQTT connection successful, publishes connection event |
| `TUYA_EVENT_MQTT_DISCONNECT` | MQTT connection lost |
| `TUYA_EVENT_TIMESTAMP_SYNC` | Cloud time synchronization |

## Related Documentation

- [TuyaOpen Official Website](https://tuyaopen.ai)
- [GitHub Repository](https://github.com/tuya/TuyaOpen)

# AI Skill Parsing

This example demonstrates how to use the **Skill** feature in the Arduino-TuyaOpen framework to parse and handle skill events dispatched by the AI. It covers music/story playback, playback control, emotion recognition, and saving TTS audio data to an SD card.

> This example only supports the TUYA_T5AI platform. It is recommended to use the [T5AI-Board Development Kit](https://tuyaopen.ai/en/docs/hardware-specific/t5-ai-board/overview-t5-ai-board) for development.

### Project File Structure

```
05_AI_Skill/
├── 05_AI_Skill.ino        # Main program entry; dispatches Skill/Emotion/TTS events in AI event callback
└── skill_handler.cpp/.h   # Skill handler module; encapsulates skill parsing, emotion handling, and TTS recording
```

## Flashing Procedure

0. Make sure you have completed the development environment setup described in [Quick Start](Quick_start.md).

1. Connect the T5AI development board to your computer, open Arduino IDE, select the `TUYA_T5AI` board, and choose the correct upload port.

> Note: T5AI series development boards provide dual serial port communication. When connected to a computer, two serial port numbers will be detected. UART0 is used for firmware flashing — please select the correct upload port in Arduino IDE.

2. In Arduino IDE, click `File` -> `Examples` -> `AI components` -> `05_AI_Skill` to open the example code.

3. Replace the authorization code and product PID in the example file with your own credentials.
    - [What is an authorization code](https://tuyaopen.ai/en/docs/quick-start#tuyaopen-authorization-code)
    - [How to obtain an authorization code](https://tuyaopen.ai/en/docs/quick-start#obtaining-tuyaopen-authorization-code)

```cpp
// Device authorization code (replace with your own)
#define TUYA_DEVICE_UUID    "uuidxxxxxxxxxxxxxxxx"
#define TUYA_DEVICE_AUTHKEY "keyxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"

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

After successful provisioning, you can converse with the AI via voice or serial text input. The following voice commands can trigger different skills:

| Voice Command Example | Triggered Skill Type | Description |
| --- | --- | --- |
| "Play music" / "Play a song" | Music Skill (MUSIC) | Returns music playback URL and metadata |
| "Tell me a story" | Story Skill (STORY) | Returns story playback URL and metadata |
| "Next" / "Pause" | Playback Control (PLAY_CONTROL) | Controls playback behavior |
| Any conversation | Emotion Recognition (EMOTION) | AI infers the emotion of the current conversation |

## Example Code Explanation

### Skill Handler Module Initialization

Call `skillInit()` in `setup()` to initialize the skill handler module and allocate the TTS data buffer (128KB):

```cpp
skillInit();
```

### AI Event Callback

This example handles the following events in `aiEventCallback`:

| Event | Description | Handling Logic |
| --- | --- | --- |
| `AI_USER_EVT_ASR_OK` | ASR speech recognition successful | Display user's spoken content on serial port |
| `AI_USER_EVT_TTS_DATA` | TTS audio data chunk | Collect to buffer via `skillCollectTts()` |
| `AI_USER_EVT_TTS_STOP` | TTS playback ended | Save buffer to SD card via `skillSaveTts()` |
| `AI_USER_EVT_TEXT_STREAM_DATA` | Text stream data | Output on serial port |
| `AI_USER_EVT_SKILL` | Skill data | Parse skill via `skillHandleEvent()` |
| `AI_USER_EVT_EMOTION` | Emotion from text tags | Handle emotion via `skillHandleEmotion()` |
| `AI_USER_EVT_LLM_EMOTION` | Emotion inferred by LLM | Handle emotion via `skillHandleEmotion()` |
| `AI_USER_EVT_PLAY_CTL_PLAY` | Playback control: Play | Serial output |
| `AI_USER_EVT_PLAY_CTL_PAUSE` | Playback control: Pause | Serial output |
| `AI_USER_EVT_PLAY_CTL_NEXT` | Playback control: Next | Serial output |
| `AI_USER_EVT_PLAY_CTL_PREV` | Playback control: Previous | Serial output |

### Skill Parsing

Use `TuyaAI.Skill.parse()` to parse skill data, supporting multiple skill types:

```cpp
void skillHandleEvent(void *data) {
    SkillData_t skill;
    if (OPRT_OK == TuyaAI.Skill.parse(data, skill)) {
        switch (skill.type) {
            case SKILL_TYPE_MUSIC:
            case SKILL_TYPE_STORY:
                // Get music/story resource information
                // skill.music->action       - Playback action
                // skill.music->src_cnt      - Number of resources
                // skill.music->src_array[i].url  - Playback URL
                TuyaAI.Skill.dumpMusic(skill);   // Print debug info
                TuyaAI.Skill.freeMusic(skill);   // Release resources (must be called)
                break;

            case SKILL_TYPE_PLAY_CONTROL:
                // skill.playControl.action  - Playback control action
                break;

            case SKILL_TYPE_EMOTION:
                // Emotion skill
                break;
        }
    }
}
```

**Skill type table:**

| Type | Macro | Description |
| --- | --- | --- |
| Music | `SKILL_TYPE_MUSIC` | Music playback request with a list of playback URLs |
| Story | `SKILL_TYPE_STORY` | Story playback request with a list of playback URLs |
| Playback Control | `SKILL_TYPE_PLAY_CONTROL` | Play / Pause / Next / Previous |
| Emotion | `SKILL_TYPE_EMOTION` | Emotion type skill |

> **Important**: Skill data of types `SKILL_TYPE_MUSIC` and `SKILL_TYPE_STORY` contains dynamically allocated resources. After processing, you must call `TuyaAI.Skill.freeMusic(skill)` to release the memory.

### Emotion Recognition

Emotion recognition during AI chat comes from two sources:

- `AI_USER_EVT_EMOTION`: Emotion parsed from text tags
- `AI_USER_EVT_LLM_EMOTION`: User emotion inferred by the large language model

Both return data via the `AI_AGENT_EMO_T` structure, containing the **emotion name** (`name`) and the **corresponding emoji** (`emoji`, in Unicode encoding):

```cpp
void skillHandleEmotion(AI_AGENT_EMO_T *emo) {
    if (!emo) return;
    Serial.print("[Emotion] ");
    Serial.print(emo->name ? emo->name : "unknown");

    if (emo->emoji) {
        char utf8[8];
        if (TuyaAI.Skill.unicodeToUtf8(emo->emoji, utf8, sizeof(utf8)) > 0) {
            Serial.print(" ");
            Serial.print(utf8);
        }
    }
    Serial.println();
}
```

> You can use `TuyaAI.Skill.unicodeToUtf8()` to convert the emoji's Unicode encoding to a UTF-8 string for display.

### TTS Audio Saving

This example saves the AI's TTS voice response to the SD card after each conversation:

1. `skillCollectTts()`: Collects TTS data into an in-memory buffer (128KB) during `AI_USER_EVT_TTS_DATA` events
2. `skillSaveTts()`: Writes the buffer data to the SD card during the `AI_USER_EVT_TTS_STOP` event

The file is saved to `/sdcard/tts_output.mp3`, and each new TTS overwrites the previous file.

## Related Documentation

- [TuyaOpen Official Website](https://tuyaopen.ai)
- [GitHub Repository](https://github.com/tuya/TuyaOpen)

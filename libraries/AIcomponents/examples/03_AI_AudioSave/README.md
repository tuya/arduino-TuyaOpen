# AI Audio Chat + Audio Recording

This example extends the AI audio chat functionality by adding the ability to save MIC microphone and TTS (Text-to-Speech) audio data to an SD card. Developers can use this feature for voice data collection, audio debugging, or offline analysis.

> This example only supports the TUYA_T5AI platform and requires SD card support. It is recommended to use the [T5AI-Board Development Kit](https://tuyaopen.ai/en/docs/hardware-specific/t5-ai-board/overview-t5-ai-board) (which includes a built-in SD card slot).

### Project File Structure

```
03_AI_AudioSave/
├── 03_AI_AudioSave.ino    # Main program entry; drives audio recording in AI event/state callbacks
└── appAudioRecord.cpp/.h  # Audio recording module; encapsulates MIC PCM and TTS MP3 storage to SD card
```

## Flashing Procedure

0. Make sure you have completed the development environment setup described in [Quick Start](Quick_start.md).

1. Connect the T5AI development board to your computer, open Arduino IDE, select the `TUYA_T5AI` board, and choose the correct upload port.

> Note: T5AI series development boards provide dual serial port communication. When connected to a computer, two serial port numbers will be detected. UART0 is used for firmware flashing — please select the correct upload port in Arduino IDE.

2. In Arduino IDE, click `File` -> `Examples` -> `AI components` -> `03_AI_AudioSave` to open the example code.

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

After successful provisioning, you can converse with the AI via voice or serial text input. Voice interaction supports four conversation modes, switchable by **double-clicking the button**. See the `02_AI_AudioChat` example for details.

## Example Code Explanation

### Hardware Configuration

```cpp
#define LED_PIN     1    // LED pin (T5AI-Board)
#define BUTTON_PIN  12   // Button pin (T5AI-Board)
#define DPID_VOLUME 3    // Volume DP ID
```

### Audio Recording Initialization

Call `appAudioRecordInit()` in `setup()` to initialize the audio recording module. This module creates an `/ai_recordings` directory on the SD card:

```cpp
appAudioRecordInit();
```

In `loop()`, periodically call `appAudioRecordFlush()` to flush the in-memory buffer data to the SD card:

```cpp
void loop() {
    handleUserInput();
    appAudioRecordFlush();  // Flush audio buffer to SD card
    delay(10);
}
```

### Audio Recording in AI Event Callback

Audio recording is driven by the AI event callback. The following events are involved:

| Event | Handling Logic |
| --- | --- |
| `AI_USER_EVT_MIC_DATA` | Write raw MIC PCM data to the buffer |
| `AI_USER_EVT_TTS_START` | Start TTS recording and create a new MP3 file |
| `AI_USER_EVT_TTS_DATA` | Write TTS MP3 data to the file |
| `AI_USER_EVT_TTS_STOP` | TTS ends normally; close the recording file |
| `AI_USER_EVT_TTS_ABORT` | TTS is interrupted; close the recording file |
| `AI_USER_EVT_TTS_ERROR` | TTS error; close the recording file |

```cpp
case AI_USER_EVT_MIC_DATA:
    if (data && len > 0) {
        appMicRecordWrite((const uint8_t*)data, len);
    }
    break;

case AI_USER_EVT_TTS_START:
    appTtsRecordStart();
    break;

case AI_USER_EVT_TTS_DATA:
    if (data && len > 0) {
        appTtsRecordWrite((const uint8_t*)data, len);
    }
    break;

case AI_USER_EVT_TTS_STOP:
    appTtsRecordStop();
    break;
```

### MIC Recording Control in AI State Callback

MIC recording start and stop is controlled by the AI state callback:

```cpp
static void aiStateCallback(AI_MODE_STATE_E state) {
    // Start MIC recording when entering the listening state
    if (state == AI_MODE_STATE_LISTEN) {
        appMicRecordStart();
    }
    // Stop MIC recording when leaving the listening state
    else if (state == AI_MODE_STATE_UPLOAD || state == AI_MODE_STATE_THINK ||
             state == AI_MODE_STATE_IDLE || state == AI_MODE_STATE_INIT) {
        appMicRecordStop();
    }
}
```

| State Change | Recording Behavior |
| --- | --- |
| Enter `LISTEN` (Listening) | Start MIC PCM recording |
| Enter `UPLOAD` / `THINK` / `IDLE` / `INIT` | Stop MIC PCM recording |

### Recorded File Description

Recording files are saved in the `/ai_recordings` directory on the SD card, with auto-incrementing sequential naming:

| File Format | Audio Format | Description |
| --- | --- | --- |
| `mic_XXX.pcm` | PCM (16kHz, 16-bit, mono) | Raw microphone audio captured while the user is speaking |
| `tts_XXX.mp3` | MP3 (cloud format) | AI TTS voice responses |

#### Playing PCM Files

MIC-recorded PCM files contain raw audio data (no file header) and can be played using ffplay:

```bash
ffplay -f s16le -ar 16000 -ac 1 mic_001.pcm
```

#### Playing MP3 Files

TTS-recorded MP3 files are in standard format and can be opened directly with any MP3 player.

### Buffering Mechanism

MIC data is accumulated in an in-memory buffer (default 16KB) before being written to the SD card in bulk, reducing frequent small write operations and improving SD card longevity and write efficiency. The buffer is periodically flushed via `appAudioRecordFlush()` called in `loop()`.

> **Note**: The SD card must be formatted as FAT32 file system. The recording directory is created automatically, and file numbering auto-increments on each boot.

## Related Documentation

- [TuyaOpen Official Website](https://tuyaopen.ai)
- [GitHub Repository](https://github.com/tuya/TuyaOpen)

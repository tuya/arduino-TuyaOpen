# AudioRecorder - Button Record & Playback

## Overview
Record audio while holding a button, then play it back after release. Double-click can stop playback.

## Features
- Press to start recording
- Release to stop recording and auto playback
- Double-click to stop playback
- LED indicator during recording

## Hardware Requirements
- TUYA_T5AI board
- Microphone connected to the board audio input
- Speaker/output connected
- Button on GPIO 12
- LED on GPIO 1 (optional)

## Pin Configuration
- `BUTTON_PIN`: GPIO 12
- `LED_PIN`: GPIO 1
- `BOARD_SPEAKER_EN_PIN`: `TUYA_GPIO_NUM_28`

## Configuration
Recording duration and buffer size:

```cpp
#define EXAMPLE_RECORD_DURATION_MS 3000
#define EXAMPLE_MIC_BUFFER_SIZE    EXAMPLE_RECORD_DURATION_MS / 10 * 640
```

Audio init parameters:

```cpp
cfg.micBufferSize = EXAMPLE_MIC_BUFFER_SIZE;
cfg.volume = 70;
cfg.spkPin = BOARD_SPEAKER_EN_PIN;
```

## How to Use
1. Upload the sketch.
2. Open Serial Monitor at 115200.
3. Press and hold the button to record (LED on).
4. Release the button to stop recording (LED off).
5. Playback starts automatically when the audio engine is idle.
6. Double-click the button to stop playback.

## Notes
- Recorded data is stored in the Audio internal buffer (no SD card).


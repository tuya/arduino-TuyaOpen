# Audio2SDcard - Record to SD (PCM/WAV)

## Overview
Record microphone audio and save it to SD card as a raw PCM file, then generate a WAV file by adding a WAV header. Recording is controlled by a button.

## Features
- Press button to start recording
- Release button to stop and save files
- Streaming write to SD during recording
- Saves both `.pcm` and `.wav`
- Auto-stop after `MAX_RECORD_MS`

## Hardware Requirements
- TUYA_T5AI board
- Microphone connected to the board audio input
- SD card (FAT32 recommended)
- Button on GPIO 12
- LED on GPIO 1 (optional)

## Pin Configuration
- `BUTTON_PIN`: GPIO 12
- `LED_PIN`: GPIO 1
- `BOARD_SPEAKER_EN_PIN`: `TUYA_GPIO_NUM_28`

## Configuration
Key macros in `Audio2SDcard.ino`:

```cpp
#define RECORDINGS_DIR       "/recordings"
#define MAX_RECORD_MS        30000
```

Audio init parameters:

```cpp
cfg.micBufferSize = 60000;
cfg.volume = 70;
cfg.spkPin = BOARD_SPEAKER_EN_PIN;
```

## How to Use
1. Insert SD card before boot.
2. Upload the sketch.
3. Open Serial Monitor at 115200.
4. Press the button to start recording (LED on).
5. Release the button to stop recording and save files (LED off).

## Output Files
Files are created under `${RECORDINGS_DIR}`:
- `rec_001.pcm` / `rec_001.wav`
- `rec_002.pcm` / `rec_002.wav`

## Notes
- Audio format: 16 kHz / 16-bit / mono.
- WAV is generated from the PCM file after recording stops.

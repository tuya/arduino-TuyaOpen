# AudioSpeaker - MP3 Playback (C Array / LittleFS / SD)

## Overview
This example demonstrates MP3 playback using the Audio library + `minimp3`. Select the MP3 source at compile time: embedded C array, LittleFS (flash), or SD card.

## Features
- MP3 decoding via `minimp3_ex`
- Source selection by macro (`AUDIO_SOURCE`)
- Unified filesystem API (`VFSFILE`) for LittleFS/SD

## Hardware Requirements
- TUYA_T5AI board
- Speaker/output enabled by `TUYA_GPIO_NUM_28`
- Optional: SD card (for SD mode)

## Configuration
Select source in `AudioSpeaker.ino`:

```cpp
// 0=C array, 1=Flash (LittleFS), 2=SD card
#define AUDIO_SOURCE 0
```

Paths used by the sketch:

```cpp
#define MP3_FILE_FLASH  "/hello_tuya.mp3"
#define MP3_FILE_SD     "/music/hello_tuya.mp3"
```

## How to Use
1. Set `AUDIO_SOURCE`.
2. If using LittleFS/SD, place your MP3 file at the path above.
3. Upload and run.
4. The sketch plays the MP3, waits 2 seconds, then repeats.

## Notes
- The example includes an embedded sample (`hello_tuya_16k.cpp`) for `AUDIO_SOURCE == 0`.
- MP3 decoding details are in `README_mp3.md`.


# Audio Output to SD Card Example

## Overview
This example demonstrates how to record audio using button control and save the recordings to an SD card in both PCM and WAV formats. It features automatic file numbering and real-time data streaming to prevent memory overflow.

## Features
- **Button-controlled recording**: Press to start, release to stop
- **Dual file format output**: Saves both PCM (raw) and WAV (with header) files
- **Auto-incrementing file names**: Files are numbered sequentially (recording_001, recording_002, etc.)
- **Real-time file writing**: Streams audio data directly to SD card during recording
- **State machine design**: Robust recording lifecycle management
- **LED indicator**: Visual feedback during recording

## Hardware Requirements
- Tuya T5AI development board (required)
- Microphone connected to audio input
- SD card inserted and formatted
- Button connected to GPIO 12
- LED connected to GPIO 1 (optional, for visual feedback)

## Pin Configuration
- **Button**: GPIO 12 (with internal pull-up)
- **LED**: GPIO 1
- **Speaker Enable**: GPIO 28 (automatically managed)

## Button Controls
- **Press**: Start recording
- **Release**: Stop recording and save files to SD card

## File Output
All recordings are saved in the `/recordings` directory on the SD card:
- `recording_001.pcm` - Raw PCM audio data
- `recording_001.wav` - PCM data with WAV header
- `recording_002.pcm`, `recording_002.wav` - Next recording
- ... and so on

## Usage Instructions

### 1. Prepare SD Card
- Format SD card as FAT32
- Insert SD card into the board before powering on

### 2. Upload the Sketch
- Open `audio_output_sdcard.ino` in Arduino IDE
- Select TUYA_T5AI as the board
- Upload the sketch

### 3. Operation
1. **Power on** the board - Serial Monitor will show initialization status
2. **Press the button** - LED turns on, recording starts
3. **Release the button** - LED turns off, files are saved to SD card
4. **Check Serial Monitor** for recording statistics and file information
5. **Repeat** to create additional recordings with auto-incremented numbers

### 4. Access Recordings
- Remove SD card from board
- Insert into computer to access `/recordings` folder
- Play WAV files with any audio player
- PCM files can be processed with audio editing software

## Audio Configuration
The example uses the following audio settings:
- **Sample Rate**: 16kHz
- **Bit Depth**: 16-bit
- **Channels**: Mono
- **Maximum Recording Duration**: 30 seconds
- **File Naming**: `recording_XXX.pcm` / `recording_XXX.wav`

## Technical Details

### State Machine
The recording process follows these states:
1. **IDLE**: Waiting for button press
2. **RECORD_START**: Opening PCM file
3. **RECORDING**: Continuously writing audio data to file
4. **RECORD_END**: Stopping recording, creating WAV file

### Real-time File Writing
Unlike buffering all audio in memory, this example streams data directly to the SD card during recording:
```cpp
void savePCMFromRingBuffer() {
    // Continuously drain ring buffer to file
    // Prevents memory overflow for long recordings
}
```

### WAV File Generation
After recording completes, the PCM file is processed to create a WAV file:
1. Read PCM file to get data size
2. Generate WAV header with audio parameters
3. Write header + PCM data to WAV file

### File Numbering
On startup, the system scans the `/recordings` directory and finds the highest existing number, then increments for the next recording.

## Serial Monitor Output Example
```
========================================
Audio Recorder to SD Card
Platform board:      TUYA_T5AI
========================================
Initializing SD card...
SD card mounted successfully
Created recordings directory
Next recording number: 1
Ready to record!

*** Start Recording ***
Recording...
Recording: 1s, 32000 bytes
Recording: 2s, 64000 bytes
Recording: 3s, 96000 bytes

*** Stop Recording ***
Recording stopped. Duration: 3045 ms, Total: 97440 bytes

=== Creating WAV file ===
PCM file size: 97440 bytes
WAV file created, data size: 97440 bytes
✓ Files saved successfully
Saved as: recording_001.pcm/.wav
Ready for next recording
```

## Troubleshooting

**SD card not detected:**
- Check SD card is inserted properly
- Verify SD card is formatted as FAT32
- Try different SD card

**No audio recorded:**
- Check microphone connection
- Verify button is connected to GPIO 12
- Check Serial Monitor for error messages

**Files not created:**
- SD card may be full - check available space
- SD card may be write-protected
- Check Serial Monitor for "Write error" messages

**Audio quality issues:**
- Check sample rate settings match your requirements
- Verify microphone is working properly
- Try recording in a quieter environment

## Code Structure
- `audio_output_sdcard.ino` - Main sketch file
- `wav_encode.h` / `wav_encode.cpp` - WAV header generation utilities
- `README.md` - This file
- `README_zh.md` - Chinese documentation

## Dependencies
- **Audio Library**: Tuya audio recording and playback
- **Button Library**: Debounced button event handling
- **FS Library**: File system operations (SD card)
- **Log Library**: Serial logging utilities

## Notes
- Only TUYA_T5AI platform is supported
- Maximum 30 seconds per recording (configurable)
- Recording number can reach 999 before wrapping
- LED blinks during file save operation

## Advanced Usage

### Customize Recording Duration
```cpp
#define MAX_RECORD_DURATION_MS 60000  // 60 seconds
```

### Change Audio Quality
```cpp
#define AUDIO_SAMPLE_RATE 8000   // Lower quality, smaller files
#define AUDIO_SAMPLE_RATE 16000  // Default
#define AUDIO_SAMPLE_RATE 48000  // Higher quality, larger files
```

### Custom File Path
```cpp
#define RECORDINGS_DIR "/my_recordings"
```

## Related Examples
- `audio_recorder` - Record and playback without SD card
- `audio_speaker` - Play MP3 files from flash or SD card
- `SDCardDemo` - Basic SD card file operations

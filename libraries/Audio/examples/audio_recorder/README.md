# Audio Recorder Example

## Overview
This example demonstrates how to use the Tuya Audio library to record and playback audio with button control and real-time audio frame processing. It showcases advanced features including audio frame callbacks for real-time data processing and manual playback control.

## Features
- **Button-controlled recording**: Start/stop recording with button press
- **Real-time audio processing**: Monitor audio level and process frames during recording
- **Automatic playback**: Recorded audio plays back automatically after recording stops
- **Audio statistics**: Track recording metrics including frame count, data size, and audio levels
- **Voice Activity Detection ready**: Frame callback can be used for VAD implementation

## Hardware Requirements
- Tuya development board with audio support
- Microphone connected to the audio input
- Speaker or audio output device connected
- Button connected to GPIO pin 12 (configurable)

## Button Controls
- **Press and Hold**: Start recording audio
- **Release**: Stop recording and automatically play back the recorded audio
- **Double Click**: Stop audio playback

## Usage Instructions

1. **Upload the sketch** to your Tuya development board
2. **Open Serial Monitor** at 115200 baud to view debug information
3. **Press and hold the button** to start recording (up to 5 seconds)
4. **Release the button** to stop recording - playback will start automatically
5. **Double-click the button** to stop playback at any time
6. **Monitor the Serial output** for recording statistics and audio levels

## Key Code Features

### 1. Audio Frame Callback
The example demonstrates real-time audio processing using `setAudioFrameCallback()`:
```cpp
void onAudioFrame(uint8_t* data, uint32_t len) {
  // Process each audio frame in real-time
  // Calculate audio level, stream to server, etc.
}
```

**Use cases:**
- Stream audio to cloud services for speech recognition
- Real-time audio level monitoring and visualization
- Voice Activity Detection (VAD)
- Real-time audio effects and filtering

### 2. Button-based Control
Uses the Button library for debounced, event-driven control:
- `BUTTON_EVENT_PRESS_DOWN`: Start recording
- `BUTTON_EVENT_PRESS_UP`: Stop recording and trigger playback
- `BUTTON_EVENT_DOUBLE_CLICK`: Stop playback

### 3. Manual Playback
Demonstrates frame-by-frame playback using `play()` interface:
```cpp
audio.play(play_buffer, out_len);
```

### 4. Audio Statistics
Tracks and displays:
- Total frames recorded
- Total bytes captured
- Maximum audio level detected

## Audio Configuration
The example uses the following audio settings (customizable):
- **Duration**: 5 seconds maximum recording time
- **Volume**: 70% playback volume
- **Sample Rate**: 16kHz
- **Format**: PCM 16-bit mono
- **Frame Size**: 640 bytes per playback frame

## Technical Details

### Audio Data Flow
1. Button press triggers recording start
2. Audio frames are captured and callback is invoked for each frame
3. Frames are automatically saved to internal ring buffer
4. Button release stops recording
5. Recorded data is read frame-by-frame and played back
6. Audio buffer is cleared after playback

### Memory Management
- Dynamic playback buffer allocation (640 bytes)
- Ring buffer for recorded audio (managed by Audio library)
- Automatic cleanup after playback

## Troubleshooting

**No audio recorded:**
- Check microphone connection
- Verify button is connected to correct GPIO pin
- Check Serial Monitor for error messages

**Playback not working:**
- Verify speaker/audio output is connected
- Check volume settings (default 70%)
- Ensure board supports audio playback

**Button not responding:**
- Verify button connection to GPIO 12
- Check button pull-up/pull-down configuration
- Review Serial Monitor for button event logs

## Example Output
```
========================================
Audio Recorder with Frame Callback Demo
Project name:        audio_recorder
...
========================================
Button initialized on pin 12
Audio initialized successfully
Ready for recording...

=== Recording Started ===
Recording: 1280 bytes, frames: 10
Recording: 2560 bytes, frames: 20
=== Recording Stopped ===
===== Audio Statistics =====
Total frames: 38
Total bytes: 4864
Max audio level: 2847
============================
Starting playback using play() interface...
Playback completed, played 38 frames
Ready for next recording...
```

## Related Examples
- Simple audio recording: See basic audio recording examples
- Audio streaming: See examples for streaming audio over network


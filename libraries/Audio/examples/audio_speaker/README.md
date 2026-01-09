# MP3 Audio Player Example

This example demonstrates MP3 audio playback from multiple sources using the Audio library and file system interfaces.

## Features

- **Multiple Audio Sources**:
  - C array data (embedded in code)
  - Internal flash storage (LittleFS)
  - External SD card (FAT filesystem)

- **MP3 Decoding**: Uses minimp3 library for efficient MP3 decoding
- **File System Integration**: Unified interface for flash and SD card access
- **Audio Playback**: PCM audio output through Audio library

## Hardware Requirements

- **Platform**: TUYA_T5AI
- **SD Card** (optional): Required only for SD card playback mode
- **Speaker**: Connected to audio output pin

## Audio File Requirements

- **Format**: MP3
- **Sample Rate**: 16kHz (must match AUDIO_SAMPLE_RATE setting)
- **Channels**: Mono or Stereo

### Converting Audio Files

You can use online tools to convert audio to the required format:
- Website: https://convertio.co/zh/
- Settings:
  - Output format: MP3
  - Sample rate: 16000 Hz
  - Bitrate: 128 kbps (recommended)

## Configuration

### Select Audio Source

Edit `AudioMP3Player.ino` and change the `AUDIO_SOURCE` macro:

```cpp
#define USE_C_ARRAY        0  // Play from embedded C array
#define USE_INTERNAL_FLASH 1  // Play from internal flash
#define USE_SD_CARD        2  // Play from SD card

#define AUDIO_SOURCE USE_C_ARRAY  // Change this value
```

### Configure File Paths

For flash and SD card modes, configure the file paths:

```cpp
#define MP3_FILE_FLASH  "/hello_tuya.mp3"      // Flash file path
#define MP3_FILE_SDCARD "/music/hello_tuya.mp3" // SD card file path
```

### Adjust Audio Settings

```cpp
#define AUDIO_SAMPLE_RATE  16000  // Must match your MP3 file
#define AUDIO_VOLUME       70     // Volume level (0-100)
```

## Usage

### Mode 1: C Array Data

1. Use the provided `hello_tuya_16k.cpp` audio data
2. Set `AUDIO_SOURCE` to `USE_C_ARRAY`
3. Upload and run

This mode is useful for:
- Fixed audio prompts
- Small audio files
- No external storage needed

### Mode 2: Internal Flash

1. Prepare your MP3 file (16kHz, MP3 format)
2. Upload the file to flash storage at `/hello_tuya.mp3`
3. Set `AUDIO_SOURCE` to `USE_INTERNAL_FLASH`
4. Upload and run

This mode is useful for:
- Larger audio files
- Updatable audio content
- No SD card required

### Mode 3: SD Card

1. Prepare your MP3 file (16kHz, MP3 format)
2. Copy the file to SD card at `/music/hello_tuya.mp3`
3. Insert SD card into the board
4. Set `AUDIO_SOURCE` to `USE_SD_CARD`
5. Upload and run

This mode is useful for:
- Very large audio files
- Multiple audio files
- Easy file management

## File System Integration

The example uses the unified file system interface:

```cpp
// For internal flash
VFSFILE fs(LITTLEFS);

// For SD card
VFSFILE fs(SDCARD);

// Common operations
if (fs.exist("/path/to/file.mp3")) {
    TUYA_FILE file = fs.open("/path/to/file.mp3", "r");
    int bytes = fs.read(buffer, size, file);
    fs.close(file);
}
```

## MP3 Decoder Integration

The example uses minimp3 for efficient MP3 decoding:

```cpp
// Initialize decoder
mp3dec_t mp3_decoder;
mp3dec_init(&mp3_decoder);

// Decode frame
int samples = mp3dec_decode_frame(&mp3_decoder,
                                 mp3_data, mp3_size,
                                 pcm_buffer, &frame_info);

// Play PCM data
audio.play((uint8_t*)pcm_buffer, pcm_size);
```

## Troubleshooting

### Audio not playing

1. Check speaker connection
2. Verify audio volume setting
3. Check serial output for error messages
4. Ensure audio file is 16kHz MP3 format

### File not found (Flash mode)

1. Verify file is uploaded to flash
2. Check file path matches `MP3_FILE_FLASH`
3. Ensure LittleFS is properly formatted

### File not found (SD Card mode)

1. Check SD card is properly inserted
2. Verify file exists at `MP3_FILE_SDCARD`
3. Check SD card mount status in serial output
4. Ensure SD card is FAT formatted

### Choppy playback

1. Increase buffer sizes if needed
2. Reduce CPU load from other tasks
3. Check audio file quality

## Example Output

```
=== MP3 Audio Player ===

Audio initialized successfully
MP3 decoder initialized
Source: C Array

--- Playing from C Array ---
Playback completed!

Playback finished!
```

## Advanced Usage

### Playing Multiple Files

Modify the code to loop through multiple files:

```cpp
const char* playlist[] = {
    "/music/track1.mp3",
    "/music/track2.mp3",
    "/music/track3.mp3"
};

for (int i = 0; i < 3; i++) {
    playFile(playlist[i]);
}
```

### Adding Playback Control

Add buttons to control playback:

```cpp
if (stopButtonPressed()) {
    player_state.playing = false;
}
```

## Related Examples

- `SDCardDemo` - SD card file operations
- `audio_speaker` - Low-level audio playback


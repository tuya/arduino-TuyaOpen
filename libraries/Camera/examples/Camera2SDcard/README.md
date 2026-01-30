# Camera2SDcard - Live View + Capture to SD

## Overview
This example shows a camera live view on the LCD (YUV422), and saves JPEG photos or H264 video to the SD card using a button.

## Modes
Select ONE mode in `Camera2SDcard.ino`:

```cpp
#define MODE_JPEG_CAPTURE
// #define MODE_H264_RECORD
```

## Controls
- Button pin: GPIO 12

### JPEG Mode (`MODE_JPEG_CAPTURE`)
- Single click: save one JPEG photo to `/sdcard/photo_<millis>.jpg`

### H264 Mode (`MODE_H264_RECORD`)
- Press down: start recording to `/sdcard/video_<millis>.h264`
- Release: stop recording and close the file

## Notes
- Live view always uses YUV422 frames.
- Saved video is raw `.h264` (no container).

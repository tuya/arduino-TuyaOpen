# Camera2Display - Live View to LCD

## Overview
This example captures camera frames in YUV422 format and displays them on the LCD in real time.

## Features
- Camera capture: 480x480 @ 15 FPS
- Live view using `display.displayYUV422Frame(...)`

## Hardware Requirements
- TUYA_T5AI board
- Camera module connected (board supported camera)
- LCD display

## Configuration
In `Camera2Display.ino`:

```cpp
#define CAMERA_FPS 15
camera.begin(CameraResolution::RES_480X480, CAMERA_FPS, CameraFormat::YUV422);
```

## How to Use
1. Upload the sketch.
2. Open Serial Monitor at 115200 for logs.
3. The LCD should show the live camera image.

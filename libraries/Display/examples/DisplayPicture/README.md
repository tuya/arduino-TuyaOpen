# DisplayPicture - Show an Image with Rotation

## Overview
This example displays a built-in image (`imga_data`) and cycles through four rotations (0/90/180/270 degrees) every 2 seconds.

## Hardware Requirements
- TUYA_T5AI board with a supported display

## How to Use
1. Upload the sketch.
2. Open Serial Monitor at 115200.
3. The image will be displayed and rotated every 2 seconds.

## Notes
- Image data is provided by the example source (e.g. `ImageData.c`).
- Rotation is applied via `display.setRotation(...)`.

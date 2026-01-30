# LVGLdemo - LVGL v9.1.0 Hello World

## Overview
This example initializes LVGL v9.1.0 via the Tuya LV vendor layer, creates a centered label ("Hello World!"), and starts the LVGL task.

## Hardware Requirements
- TUYA_T5AI board with a supported display

## How to Use
1. Upload the sketch.
2. The screen shows a white background with a centered "Hello World!" label.

## Notes
- LVGL task is started by:

```cpp
lv_vendor_start(5, 1024*8);
```

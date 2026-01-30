# Camera2SDcard - 预览 + 保存到 SD

## 概述
本示例在 LCD 上显示摄像头实时预览（YUV422），并通过按键将 JPEG 照片或 H264 视频保存到 SD 卡。

## 模式选择
在 `Camera2SDcard.ino` 中二选一：

```cpp
#define MODE_JPEG_CAPTURE
// #define MODE_H264_RECORD
```

## 按键控制
- 按键引脚：GPIO 12

### JPEG 模式（`MODE_JPEG_CAPTURE`）
- 单击：保存一张照片到 `/sdcard/photo_<millis>.jpg`

### H264 模式（`MODE_H264_RECORD`）
- 按下：开始录像，保存到 `/sdcard/video_<millis>.h264`
- 松开：停止录像并关闭文件

## 说明
- 实时预览始终使用 YUV422。
- H264 模式保存的是原始 `.h264` 数据（无封装容器）。

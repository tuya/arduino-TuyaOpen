# Camera2Display - 摄像头预览到屏幕

## 概述
本示例将摄像头采集的 YUV422 图像实时显示到 LCD 屏幕。

## 功能特性
- 摄像头采集：480x480 @ 15 FPS
- 使用 `display.displayYUV422Frame(...)` 直接显示

## 硬件要求
- TUYA_T5AI 开发板
- 摄像头模组（板级支持的摄像头）
- LCD 显示屏

## 配置
在 `Camera2Display.ino` 中：

```cpp
#define CAMERA_FPS 15
camera.begin(CameraResolution::RES_480X480, CAMERA_FPS, CameraFormat::YUV422);
```

## 使用方法
1. 上传示例程序。
2. 打开串口监视器（115200）查看日志。
3. 屏幕应显示实时预览画面。

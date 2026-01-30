# AudioSpeaker - MP3 播放（C 数组 / LittleFS / SD）

## 概述
本示例演示使用 Audio 库配合 `minimp3` 解码并播放 MP3。MP3 的来源可通过宏选择：内置 C 数组、Flash（LittleFS）或 SD 卡。

## 功能特性
- 使用 `minimp3_ex` 解码 MP3
- 通过宏 `AUDIO_SOURCE` 选择音频来源
- 使用统一文件系统接口 `VFSFILE` 读取 LittleFS/SD

## 硬件要求
- TUYA_T5AI 开发板
- 扬声器/音频输出（使能脚：`TUYA_GPIO_NUM_28`）
- 可选：SD 卡（仅 SD 模式需要）

## 配置
在 `AudioSpeaker.ino` 中选择来源：

```cpp
// 0=C array, 1=Flash (LittleFS), 2=SD card
#define AUDIO_SOURCE 0
```

文件路径：

```cpp
#define MP3_FILE_FLASH  "/hello_tuya.mp3"
#define MP3_FILE_SD     "/music/hello_tuya.mp3"
```

## 使用方法
1. 设置 `AUDIO_SOURCE`。
2. 如果使用 LittleFS/SD，将 MP3 文件放到上面的路径。
3. 上传并运行。
4. 程序播放一次 MP3，延时 2 秒后循环播放。

## 说明
- `AUDIO_SOURCE == 0` 时会播放示例内置音频（`hello_tuya_16k.cpp`）。
- MP3 解码细节参见 `README_mp3.md`。

# Audio2SDcard - 录音保存到 SD（PCM/WAV）

## 概述
本示例演示如何采集麦克风音频并保存到 SD 卡：录音过程中实时写入 `.pcm` 文件，停止录音后再生成带 WAV 头的 `.wav` 文件。录音由按键控制。

## 功能特性
- 按键控制：按下开始录音，松开停止并保存
- 录音过程中流式写 SD（降低内存压力）
- 同时输出 `.pcm` 与 `.wav`
- 支持最长录音时长自动停止

## 硬件要求
- TUYA_T5AI 开发板
- 麦克风接入板载音频输入
- SD 卡（建议 FAT32）
- 按键接 GPIO 12
- LED 接 GPIO 1（可选）

## 引脚配置
- `BUTTON_PIN`：GPIO 12
- `LED_PIN`：GPIO 1
- `BOARD_SPEAKER_EN_PIN`：`TUYA_GPIO_NUM_28`

## 配置项
`Audio2SDcard.ino` 中关键宏：

```cpp
#define RECORDINGS_DIR       "/recordings"
#define MAX_RECORD_MS        30000
```

音频初始化参数：

```cpp
cfg.micBufferSize = 60000;
cfg.volume = 70;
cfg.spkPin = BOARD_SPEAKER_EN_PIN;
```

## 使用方法
1. 上电前插入 SD 卡。
2. 上传示例程序。
3. 打开串口监视器（115200）。
4. 按下按键开始录音（LED 亮）。
5. 松开按键停止录音并保存文件（LED 灭）。

## 输出文件
文件保存在 `${RECORDINGS_DIR}` 下：
- `rec_001.pcm` / `rec_001.wav`
- `rec_002.pcm` / `rec_002.wav`

## 说明
- 音频格式：16 kHz / 16-bit / 单声道。
- WAV 文件在停止录音后由 PCM 生成。


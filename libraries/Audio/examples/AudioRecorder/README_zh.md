# AudioRecorder - 按键录音与回放

## 概述
本示例演示按住按键录音、松开按键停止录音并自动回放，并支持“双击按键停止播放”。

## 功能特性
- 按下开始录音
- 松开停止录音并自动回放
- 双击停止播放
- 录音时 LED 指示

## 硬件要求
- TUYA_T5AI 开发板
- 麦克风接入板载音频输入
- 扬声器/音频输出
- 按键接 GPIO 12
- LED 接 GPIO 1（可选）

## 引脚配置
- `BUTTON_PIN`：GPIO 12
- `LED_PIN`：GPIO 1
- `BOARD_SPEAKER_EN_PIN`：`TUYA_GPIO_NUM_28`

## 配置项
录音时长与缓冲区大小：

```cpp
#define EXAMPLE_RECORD_DURATION_MS 3000
#define EXAMPLE_MIC_BUFFER_SIZE    EXAMPLE_RECORD_DURATION_MS / 10 * 640
```

音频初始化参数：

```cpp
cfg.micBufferSize = EXAMPLE_MIC_BUFFER_SIZE;
cfg.volume = 70;
```

## 使用方法
1. 上传示例程序。
2. 打开串口监视器（115200）。
3. 按住按键开始录音（LED 亮）。
4. 松开按键停止录音（LED 灭）。
5. 当音频引擎空闲时会自动开始回放。
6. 双击按键可停止播放。

## 说明
- 录制数据保存在 Audio 库的内部缓冲区中（无需 SD 卡）。


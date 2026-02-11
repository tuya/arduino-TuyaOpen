# AI 语音聊天 + 音频录制

本示例在 AI 语音聊天的基础上，新增了将 MIC 麦克风和 TTS 语音合成的音频数据保存到 SD 卡的功能。开发者可利用此功能进行语音数据采集、音频调试或离线分析。

> 本示例仅支持 TUYA_T5AI 平台，且需要 SD 卡支持。推荐使用 [T5AI-Board 开发套件](https://tuyaopen.ai/zh/docs/hardware-specific/t5-ai-board/overview-t5-ai-board)（自带 SD 卡槽）。

### 项目文件结构

```
03_AI_AudioSave/
├── 03_AI_AudioSave.ino    # 主程序入口，AI 事件/状态回调中驱动音频录制
└── appAudioRecord.cpp/.h  # 音频录制模块，封装 MIC PCM 和 TTS MP3 的 SD 卡存储
```

## 代码烧录流程

0. 确保已完成[快速开始](Quick_start.md)中开发环境的搭建。

1. 连接 T5AI 开发板到电脑，打开 Arduino IDE，选择 `TUYA_T5AI` 开发板，并选择正确的烧录端口。

> 注意：T5AI 系列开发板提供双串口通信，连接电脑后会检测到两个串口号，其中 UART0 用于固件烧录，请在 Arduino IDE 中选择正确的烧录口。

2. 在 Arduino IDE 中点击 `文件` -> `示例` -> `AI components` -> `03_AI_AudioSave`，打开示例代码。

3. 将示例文件中的授权码信息和产品 PID 替换为自己的信息。
    - [什么是授权码](https://tuyaopen.ai/zh/docs/quick-start#tuyaopen-%E4%B8%93%E7%94%A8%E6%8E%88%E6%9D%83%E7%A0%81)
    - [如何获取授权码](https://tuyaopen.ai/zh/docs/quick-start#tuyaopen-%E6%8E%88%E6%9D%83%E7%A0%81%E8%8E%B7%E5%8F%96)

```cpp
// 设备授权码（替换为自己的授权码）
#define TUYA_DEVICE_UUID    "uuidxxxxxxxxxxxxxxxx"
#define TUYA_DEVICE_AUTHKEY "keyxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"

```

4. 点击 Arduino IDE 左上角按钮烧录代码，终端出现以下信息说明烧录成功。

```bash
[INFO]: Write flash success
[INFO]: CRC check success
[INFO]: Reboot done
[INFO]: Flash write success.
```

## 设备连接与交互

### 设备连接

烧录固件后的开发板需要连接网络并注册到云端。

设备配网流程：[设备配网](https://tuyaopen.ai/zh/docs/quick-start/device-network-configuration)

> **快速重新配网**：短时间内连续按下 3 次复位键，设备将清除已保存的网络配置并重新进入配网状态。

### 设备交互

配网成功后，可通过语音或串口文本与 AI 进行对话。语音交互支持四种对话模式，通过**双击按键**切换。详见 `02_AI_AudioChat` 示例。

## 示例代码说明

### 硬件配置

```cpp
#define LED_PIN     1    // LED 引脚（T5AI-Board）
#define BUTTON_PIN  12   // 按键引脚（T5AI-Board）
#define DPID_VOLUME 3    // 音量 DP ID
```

### 音频录制初始化

在 `setup()` 中调用 `appAudioRecordInit()` 初始化音频录制模块。该模块会在 SD 卡上创建 `/ai_recordings` 录音目录：

```cpp
appAudioRecordInit();
```

在 `loop()` 中需定时调用 `appAudioRecordFlush()` 将内存缓冲区中的数据写入 SD 卡：

```cpp
void loop() {
    handleUserInput();
    appAudioRecordFlush();  // 刷新音频缓冲区到 SD 卡
    delay(10);
}
```

### AI 事件回调中的音频录制

音频录制由 AI 事件回调驱动，主要涉及以下事件：

| 事件 | 处理逻辑 |
| --- | --- |
| `AI_USER_EVT_MIC_DATA` | 将 MIC 原始 PCM 数据写入缓冲区 |
| `AI_USER_EVT_TTS_START` | 开始 TTS 录制，创建新的 MP3 文件 |
| `AI_USER_EVT_TTS_DATA` | 将 TTS MP3 数据写入文件 |
| `AI_USER_EVT_TTS_STOP` | TTS 正常结束，关闭录制文件 |
| `AI_USER_EVT_TTS_ABORT` | TTS 被中断，关闭录制文件 |
| `AI_USER_EVT_TTS_ERROR` | TTS 出错，关闭录制文件 |

```cpp
case AI_USER_EVT_MIC_DATA:
    if (data && len > 0) {
        appMicRecordWrite((const uint8_t*)data, len);
    }
    break;

case AI_USER_EVT_TTS_START:
    appTtsRecordStart();
    break;

case AI_USER_EVT_TTS_DATA:
    if (data && len > 0) {
        appTtsRecordWrite((const uint8_t*)data, len);
    }
    break;

case AI_USER_EVT_TTS_STOP:
    appTtsRecordStop();
    break;
```

### AI 状态回调中的 MIC 录制控制

MIC 录制的启停由 AI 状态回调控制：

```cpp
static void aiStateCallback(AI_MODE_STATE_E state) {
    // 进入聆听状态时开始 MIC 录制
    if (state == AI_MODE_STATE_LISTEN) {
        appMicRecordStart();
    }
    // 离开聆听状态时停止 MIC 录制
    else if (state == AI_MODE_STATE_UPLOAD || state == AI_MODE_STATE_THINK ||
             state == AI_MODE_STATE_IDLE || state == AI_MODE_STATE_INIT) {
        appMicRecordStop();
    }
}
```

| 状态变化 | 录制行为 |
| --- | --- |
| 进入 `LISTEN`（聆听） | 开始 MIC PCM 录制 |
| 进入 `UPLOAD` / `THINK` / `IDLE` / `INIT` | 停止 MIC PCM 录制 |

### 录制文件说明

录音文件保存在 SD 卡的 `/ai_recordings` 目录下，按序号自动递增命名：

| 文件格式 | 音频格式 | 说明 |
| --- | --- | --- |
| `mic_XXX.pcm` | PCM（16kHz, 16bit, 单声道） | 用户说话时的麦克风原始音频 |
| `tts_XXX.mp3` | MP3（云端格式） | AI TTS 语音回复 |

#### 播放 PCM 文件

MIC 录制的 PCM 文件为原始音频数据（无文件头），可使用 ffplay 播放：

```bash
ffplay -f s16le -ar 16000 -ac 1 mic_001.pcm
```

#### 播放 MP3 文件

TTS 录制的 MP3 文件为标准格式，可直接使用任意 MP3 播放器打开。

### 缓冲机制

MIC 数据采用内存缓冲区（默认 16KB）累积后再批量写入 SD 卡，以减少频繁的小块写入操作，提升 SD 卡使用寿命和写入效率。缓冲区通过 `loop()` 中的 `appAudioRecordFlush()` 定时刷新。

> **注意**：SD 卡需格式化为 FAT32 文件系统。录音目录会自动创建，文件序号每次开机自动递增。

## 相关文档

- [TuyaOpen 官网](https://tuyaopen.ai)
- [Github 代码库](https://github.com/tuya/TuyaOpen)

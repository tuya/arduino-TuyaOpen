# AI 语音聊天

本示例演示如何使用 Arduino-TuyaOpen 框架实现完整的 AI 语音聊天功能。用户可通过语音或串口文本与 AI 大模型进行对话，支持多种对话模式切换、按键交互以及云端 DP 音量控制。

> 本示例仅支持 TUYA_T5AI 平台。推荐使用 [T5AI-Board 开发套件](https://tuyaopen.ai/zh/docs/hardware-specific/t5-ai-board/overview-t5-ai-board) 进行开发。

## 代码烧录流程

0. 确保已完成[快速开始](Quick_start.md)中开发环境的搭建。

1. 连接 T5AI 开发板到电脑，打开 Arduino IDE，选择 `TUYA_T5AI` 开发板，并选择正确的烧录端口。

> 注意：T5AI 系列开发板提供双串口通信，连接电脑后会检测到两个串口号，其中 UART0 用于固件烧录，请在 Arduino IDE 中选择正确的烧录口。

2. 在 Arduino IDE 中点击 `文件` -> `示例` -> `AI components` -> `02_AI_AudioChat`，打开示例代码。

3. 将示例文件中的授权码信息和产品 PID 替换为自己的信息。
    - [什么是授权码](https://tuyaopen.ai/zh/docs/quick-start#tuyaopen-%E4%B8%93%E7%94%A8%E6%8E%88%E6%9D%83%E7%A0%81)
    - [如何获取授权码](https://tuyaopen.ai/zh/docs/quick-start#tuyaopen-%E6%8E%88%E6%9D%83%E7%A0%81%E8%8E%B7%E5%8F%96)

```cpp
// 设备授权码（替换为自己的授权码）
#define TUYA_DEVICE_UUID    "uuidxxxxxxxxxxxxxxxx"
#define TUYA_DEVICE_AUTHKEY "keyxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
// 产品 PID（替换为涂鸦 IoT 平台上创建的产品 PID）
#define TUYA_PRODUCT_ID     "xxxxxxxxxxxxxxxx"
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

烧录固件后的开发板需要连接网络并注册到云端，使设备具备远程通信能力。配网后即可实现 `手机` - `涂鸦云` - `设备` 三方通信。

设备配网流程：[设备配网](https://tuyaopen.ai/zh/docs/quick-start/device-network-configuration)

> **快速重新配网**：短时间内连续按下 3 次复位键，设备将清除已保存的网络配置并重新进入配网状态。

### 设备交互

手机 APP 配网成功后，可以通过以下两种方式与 AI Agent 进行交互。

#### 文本交互

打开 Arduino IDE 自带的串口监视器（波特率 115200），在输入框中键入文本后按回车发送，即可通过串口与 AI Agent 进行文字对话。

#### 语音交互

语音交互支持四种对话模式，默认配置为 `唤醒对话` 模式。用户可通过**双击按键**在各模式间循环切换。

**唤醒词**：你好涂鸦（hey tuya）

| 模式编号 | 模式名称 | 说明 |
| :---: | :---: | --- |
| 0 | 长按对话（HOLD） | 用户需长按按键以启动语音输入，松开按键即结束语音输入 |
| 1 | 按键对话（ONE_SHOT） | 设备上电默认进入待机状态，用户短按按键后设备进入聆听状态，可进行单次语音对话 |
| 2 | 唤醒对话（WAKEUP） | 用户通过唤醒词将设备唤醒后，可进行单次对话，类似智能音箱交互 |
| 3 | 自由对话（FREE） | 用户通过唤醒词唤醒设备后可进行连续对话；若唤醒后 30 秒无对话，设备将再次进入待机状态 |

## 示例代码说明

### 硬件配置

```cpp
#define LED_PIN     9    // LED 引脚
#define BUTTON_PIN  29   // 按键引脚
#define DPID_VOLUME 3    // 音量 DP ID
```

### AI 初始化

```cpp
// 配置 AI 核心参数
AIConfig_t aiConfig = {AI_CHAT_MODE_WAKEUP, 70, aiEventCallback, aiStateCallback, nullptr};
TuyaAI.begin(aiConfig);
// 初始化音频子系统
TuyaAI.Audio.begin();
```

与 `01_AI_TextChat` 示例不同，本示例同时注册了 `aiEventCallback` 和 `aiStateCallback` 两个回调函数，以支持完整的语音交互流程。

### AI 事件回调

```cpp
static void aiEventCallback(AI_USER_EVT_TYPE_E event, uint8_t *data, uint32_t len, void *arg);
```

本示例处理以下 AI 事件：

| 事件 | 说明 | 回调数据 |
| --- | --- | --- |
| `AI_USER_EVT_ASR_OK` | ASR 语音识别成功 | 识别出的文本内容 |
| `AI_USER_EVT_TEXT_STREAM_START` | 大模型文本回复流开始 | 首段文本数据 |
| `AI_USER_EVT_TEXT_STREAM_DATA` | 大模型文本回复流数据 | 文本数据块 |

> 当用户通过语音进行对话时，云端 ASR 识别出的文本会通过 `AI_USER_EVT_ASR_OK` 事件返回，方便开发者在串口或屏幕上显示用户所说的内容。

### AI 状态回调

```cpp
static void aiStateCallback(AI_MODE_STATE_E state);
```

AI 设备在运行过程中会在不同状态间切换，`aiStateCallback` 负责接收状态变化通知。状态列表如下：

| 状态 | 说明 |
| --- | --- |
| `AI_MODE_STATE_IDLE` | 待机状态 |
| `AI_MODE_STATE_INIT` | 初始化中 |
| `AI_MODE_STATE_LISTEN` | 聆听中（正在采集语音） |
| `AI_MODE_STATE_UPLOAD` | 上传中（语音数据上传云端） |
| `AI_MODE_STATE_THINK` | 思考中（等待大模型回复） |
| `AI_MODE_STATE_SPEAK` | 播报中（TTS 正在播放） |

### 按键交互

使用 `Button` 组件对按键功能进行封装。默认使用 GPIO 29 引脚，低电平触发，内部上拉。

支持以下按键事件：

| 事件 | 行为 |
| --- | --- |
| 按下 / 抬起 | 委托给 `TuyaAI.modeKeyHandle()` 处理 |
| 单击（`BUTTON_EVENT_SINGLE_CLICK`） | 委托给 `TuyaAI.modeKeyHandle()` 处理，触发当前模式下的对话行为 |
| 双击（`BUTTON_EVENT_DOUBLE_CLICK`） | 停止音频播放 → 中断当前对话 → 切换到下一个对话模式 → 播放模式提示音 |
| 长按（`BUTTON_EVENT_LONG_PRESS_START`） | 委托给 `TuyaAI.modeKeyHandle()` 处理（长按对话模式下启动录音） |

双击切换模式的核心逻辑：

```cpp
if (event == BUTTON_EVENT_DOUBLE_CLICK) {
    TuyaAI.Audio.stop();           // 停止当前音频播放
    TuyaAI.interruptChat();        // 中断当前对话
    AI_CHAT_MODE_E nextMode = TuyaAI.nextChatMode();  // 获取下一个模式
    TuyaAI.saveModeConfig(nextMode, volume);           // 保存模式配置
    // 播放对应模式的提示音
    AI_AUDIO_ALERT_TYPE_E alert = (AI_AUDIO_ALERT_TYPE_E)(AI_AUDIO_ALERT_LONG_KEY_TALK + (int)nextMode);
    TuyaAI.Audio.playAlert(alert);
}
```

### DP 音量控制

本示例通过 DP（DPID = 3）实现云端音量控制：

```cpp
// 接收 APP 下发的音量值
case DPID_VOLUME:
    uint8_t volume = dp->value.dp_value;
    TuyaAI.setVolume(volume);
    break;
```

设备首次连接 MQTT 后会自动上报当前音量：

```cpp
case TUYA_EVENT_MQTT_CONNECTED:
    if (gFirstConnect) {
        gFirstConnect = false;
        aiAudioVolumUpload();  // 上报当前音量到云端
    }
    break;
```

## 相关文档

- [TuyaOpen 官网](https://tuyaopen.ai)
- [Github 代码库](https://github.com/tuya/TuyaOpen)

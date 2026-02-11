# AI 文本聊天

本示例演示如何使用 Arduino-TuyaOpen 框架实现最精简的 AI 文本聊天功能。用户可通过串口输入文本与云端 AI 大模型进行对话，AI 的回复将以流式文本的形式实时输出到串口监视器。

> 本示例是所有 AI 示例中最精简的入门示例，仅包含文本交互的核心逻辑，无需按键、UI 或音频交互。

## 代码烧录流程

0. 确保已完成[快速开始](Quick_start.md)中开发环境的搭建。

1. 连接 T5AI 开发板到电脑，打开 Arduino IDE，选择 `TUYA_T5AI` 开发板，并选择正确的烧录端口。

> 注意：T5AI 系列开发板提供双串口通信，连接电脑后会检测到两个串口号，其中 UART0 用于固件烧录，请在 Arduino IDE 中选择正确的烧录口。

2. 在 Arduino IDE 中点击 `文件` -> `示例` -> `AI components` -> `01_AI_TextChat`，打开示例代码。

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

烧录固件后的开发板需要连接网络并注册到云端。

设备配网流程：[设备配网](https://tuyaopen.ai/zh/docs/quick-start/device-network-configuration)

> **快速重新配网**：短时间内连续按下 3 次复位键，设备将清除已保存的网络配置并重新进入配网状态。

### 文本交互

打开 Arduino IDE 自带的串口监视器（波特率 115200），在输入框中键入文本后按回车发送，即可通过串口与 AI 大模型进行文字对话。

交互示例：

```
[User]: 今天天气怎么样？
[AI]: 今日天气晴朗，气温适宜...

[User]: 讲个笑话
[AI]: 为什么程序员喜欢用黑色主题？因为 Bug 在黑暗中更容易隐藏！
```

## 示例代码说明

### AI 初始化

```cpp
// 配置 AI 核心参数
AIConfig_t cfg = {AI_CHAT_MODE_WAKEUP, 70, aiEventCallback, nullptr, nullptr};
TuyaAI.begin(cfg);
// 初始化音频子系统（TTS 播放需要）
TuyaAI.Audio.begin();
```

**参数说明：**

| 参数 | 说明 |
| --- | --- |
| `AI_CHAT_MODE_WAKEUP` | 初始对话模式（本示例仅使用文本，此参数不影响文本交互） |
| `70` | 初始音量（范围 0 ~ 100） |
| `aiEventCallback` | AI 事件回调函数，用于接收 AI 回复的文本流 |
| `nullptr` | AI 状态回调函数（本示例不使用） |
| `nullptr` | 用户自定义数据指针 |

> 本示例中状态回调传入 `nullptr`，这是因为纯文本聊天场景不需要监听设备状态变化。当需要更复杂的交互（如语音、UI）时，应注册状态回调。

### 发送文本

通过 `TuyaAI.sendText()` 将串口接收到的文本发送到云端大模型：

```cpp
static void handleUserInput(void) {
    static int i = 0;
    static uint8_t _recv_buf[256];
    while (Serial.available()) {
        char c = Serial.read();
        _recv_buf[i++] = c;
        if (c == '\n' || c == '\r') {
            TuyaAI.sendText(_recv_buf, i);
            i = 0;
            memset(_recv_buf, 0, sizeof(_recv_buf));
        }
    }
}
```

### AI 事件回调

本示例仅处理 **文本流（TEXT_STREAM）** 相关的三个事件：

```cpp
static void aiEventCallback(AI_USER_EVT_TYPE_E event, uint8_t *data, uint32_t len, void *arg)
{
    switch (event) {
        case AI_USER_EVT_TEXT_STREAM_START:
            Serial.print("[AI]: ");
            if (data && len > 0) Serial.write(data, len);
            break;
        case AI_USER_EVT_TEXT_STREAM_DATA:
            if (data && len > 0) Serial.write(data, len);
            break;
        case AI_USER_EVT_TEXT_STREAM_STOP:
            Serial.println("\n");
            break;
        default:
            break;
    }
}
```

| 事件 | 说明 | 回调数据 |
| --- | --- | --- |
| `AI_USER_EVT_TEXT_STREAM_START` | 大模型文本回复流开始 | 首段文本数据 |
| `AI_USER_EVT_TEXT_STREAM_DATA` | 大模型文本回复流数据 | 文本数据块 |
| `AI_USER_EVT_TEXT_STREAM_STOP` | 大模型文本回复流结束 | 无 |

> AI 的文本回复采用流式传输，`START` 事件可能携带首段文本数据，随后通过多次 `DATA` 事件逐步返回完整回复，最后以 `STOP` 事件标志回复结束。

### IoT 事件处理

本示例的 IoT 事件处理较为精简，仅处理配网和连接相关的基础事件：

| 事件 | 说明 |
| --- | --- |
| `TUYA_EVENT_BIND_START` | 设备开始配网，播放配网提示音 |
| `TUYA_EVENT_MQTT_CONNECTED` | MQTT 连接成功，发布连接事件 |
| `TUYA_EVENT_MQTT_DISCONNECT` | MQTT 连接断开 |
| `TUYA_EVENT_TIMESTAMP_SYNC` | 云端时间同步 |

## 相关文档

- [TuyaOpen 官网](https://tuyaopen.ai)
- [Github 代码库](https://github.com/tuya/TuyaOpen)

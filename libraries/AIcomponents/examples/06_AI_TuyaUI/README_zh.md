# AI 自定义 UI 显示

本示例演示如何使用 Arduino-TuyaOpen 框架中的 **TuyaAI.UI** 模块配合 **LVGL** 图形库，在 T5AI 开发板的屏幕上创建自定义的 AI 聊天界面。用户的语音文本和 AI 的回复将实时显示在屏幕上，同时展示设备状态和 WiFi 连接信息。

> 本示例仅支持带显示屏的 TUYA_T5AI 平台。推荐使用 [T5AI-Board 开发套件](https://tuyaopen.ai/zh/docs/hardware-specific/t5-ai-board/overview-t5-ai-board) 进行开发。

### 项目文件结构

```
06_AI_TuyaUI/
├── 06_AI_TuyaUI.ino       # 主程序入口，AI 事件/状态回调中驱动 UI 更新
└── ui_display.cpp/.h      # UI 显示模块，封装 LVGL 组件的创建和文本更新
```

## 代码烧录流程

0. 确保已完成[快速开始](Quick_start.md)中开发环境的搭建。

1. 连接 T5AI 开发板到电脑，打开 Arduino IDE，选择 `TUYA_T5AI` 开发板，并选择正确的烧录端口。

> 注意：T5AI 系列开发板提供双串口通信，连接电脑后会检测到两个串口号，其中 UART0 用于固件烧录，请在 Arduino IDE 中选择正确的烧录口。

2. 在 Arduino IDE 中点击 `文件` -> `示例` -> `AI components` -> `06_AI_TuyaUI`，打开示例代码。

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

### 设备交互

配网成功后，可通过语音或串口文本与 AI 进行对话。屏幕会实时显示：
- **用户文本**（蓝色）：语音识别结果或串口输入内容
- **AI 回复**（白色）：大模型的文本回复（流式更新）
- **设备状态**（绿色）：当前 AI 工作状态（Ready / Listening / Thinking 等）
- **WiFi 图标**：网络连接状态

## 示例代码说明

### UI 初始化

本示例使用 `BOT_UI_USER` 模式，仅初始化 LVGL 图形引擎，由用户自行设计 UI 界面：

```cpp
// 初始化 Display 组件
gDisplay.begin();
gDisplay.setBrightness(80);

// 初始化 AI UI 子系统（用户自定义模式）
TuyaAI.UI.begin(BOT_UI_USER);

// 创建自定义 LVGL UI
uiInit();
```

**UI 模式说明：**

| 模式 | 说明 |
| --- | --- |
| `BOT_UI_WECHAT` | 系统内置 WeChat 风格 UI，开箱即用 |
| `BOT_UI_USER` | 仅初始化 LVGL 引擎，用户自行设计 UI |

### UI 布局

`ui_display.cpp` 中的 `uiInit()` 函数创建了一个极简暗色主题的聊天界面：

```
┌──────────────────────────┐
│ [Ready]          [WiFi]  │ ← 顶部状态栏
├──────────────────────────┤
│ User: (用户语音文本)      │ ← 用户文本（蓝色 #4fc3f7）
│                          │
│ AI: (AI 回复文本)         │ ← AI 文本（白色 #ffffff）
│                          │
└──────────────────────────┘
     背景色: #1a1a2e
```

UI 组件使用 `TuyaAI.UI.getTextFont()` 获取系统提供的中英文字体，确保中文显示正常。

### AI 事件回调驱动 UI 更新

在 `aiEventCallback` 中，将 AI 产生的数据实时更新到屏幕：

| 事件 | UI 更新逻辑 |
| --- | --- |
| `AI_USER_EVT_ASR_OK` | 调用 `uiSetUser(text)` 显示用户语音文本 |
| `AI_USER_EVT_TEXT_STREAM_START` | 调用 `uiResetAIBuffer()` 重置 AI 文本缓冲区，开始追加 |
| `AI_USER_EVT_TEXT_STREAM_DATA` | 调用 `uiAppendAIBuffer()` 追加文本并实时更新 AI 标签 |
| `AI_USER_EVT_TEXT_STREAM_STOP` | 调用 `uiFlushAIBuffer()` 完成本次回复 |

```cpp
case AI_USER_EVT_ASR_OK:
    if (data && len > 0) {
        char text[256];
        int n = (len < sizeof(text) - 1) ? len : sizeof(text) - 1;
        memcpy(text, data, n);
        text[n] = '\0';
        uiSetUser(text);   // 更新用户文本
    }
    break;

case AI_USER_EVT_TEXT_STREAM_START:
    uiResetAIBuffer();              // 重置 AI 缓冲区
    uiAppendAIBuffer(data, len);    // 追加首段文本
    break;

case AI_USER_EVT_TEXT_STREAM_DATA:
    uiAppendAIBuffer(data, len);    // 追加文本数据
    break;

case AI_USER_EVT_TEXT_STREAM_STOP:
    uiFlushAIBuffer();              // 完成回复
    break;
```

> AI 的文本回复通过缓冲区机制实现流式显示：每次收到新的文本块时追加到缓冲区并刷新 LVGL 标签，用户可以看到 AI 回复逐字出现的效果。

### AI 状态回调驱动状态显示

在 `aiStateCallback` 中更新屏幕上的状态文本：

```cpp
static void aiStateCallback(AI_MODE_STATE_E state) {
    const char *s = "Ready";
    switch (state) {
        case AI_MODE_STATE_LISTEN: s = "Listening..."; break;
        case AI_MODE_STATE_UPLOAD: s = "Processing..."; break;
        case AI_MODE_STATE_THINK:  s = "Thinking..."; break;
        case AI_MODE_STATE_SPEAK:  s = "Speaking..."; break;
        default: break;
    }
    uiSetStatus(s);
}
```

### UI 显示接口

`ui_display.h` 中封装了以下接口：

| 接口 | 说明 |
| --- | --- |
| `uiInit()` | 初始化 UI，创建 LVGL 组件 |
| `uiSetUser(text)` | 显示用户文本 |
| `uiSetAI(text)` | 显示 AI 回复文本 |
| `uiSetStatus(text)` | 更新设备状态文本 |
| `uiSetWifi(connected)` | 更新 WiFi 状态图标颜色 |
| `uiResetAIBuffer()` | 重置 AI 文本缓冲区 |
| `uiAppendAIBuffer(data, len)` | 追加 AI 文本到缓冲区并刷新显示 |
| `uiFlushAIBuffer()` | 完成本次 AI 回复 |

### TuyaAI.UI 资源接口

`TuyaAI.UI` 提供以下字体和图标资源，方便用户自定义 UI 时使用：

| 接口 | 说明 |
| --- | --- |
| `TuyaAI.UI.getTextFont()` | 获取中英文文本字体 |
| `TuyaAI.UI.getIconFont()` | 获取 FontAwesome 图标字体 |
| `TuyaAI.UI.getEmoFont()` | 获取表情字体 |
| `TuyaAI.UI.getEmoList()` | 获取表情列表 |
| `TuyaAI.UI.getWifiIcon(status)` | 获取 WiFi 状态图标字符 |
| `TuyaAI.UI.displayText(text, isUser)` | 通过内置接口显示文本 |
| `TuyaAI.UI.displayWifiStatus(status)` | 通过内置接口显示 WiFi 状态 |
| `TuyaAI.UI.displayChatMode(mode)` | 通过内置接口显示对话模式 |

## 相关文档

- [TuyaOpen 官网](https://tuyaopen.ai)
- [Github 代码库](https://github.com/tuya/TuyaOpen)

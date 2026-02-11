# 应用开发 - AI 聊天机器人

Arduino-TuyaOpen 提供了一个完整的 AI 聊天机器人应用实例。用户可以基于 `YourChatBot` 示例代码，配合 TUYA-T5AI 系列开发板，快速构建一个支持语音交互、文本对话、MCP 工具调用等功能的智能聊天机器人。

### 项目文件结构

```
YourChatBot/
├── YourChatBot.ino       # 主程序入口，包含 setup/loop、AI 事件回调、IoT 事件回调
├── appButton.cpp/.h      # 按键交互模块，处理单击/双击/长按等按键事件
├── appDisplay.cpp/.h     # UI 显示模块，支持内置 WeChat 风格 UI 和自定义 LVGL UI
├── appMCP.cpp/.h         # MCP 工具注册模块，演示拍照/音量/模式切换/设备信息等工具
├── appStatus.cpp/.h      # 状态监控模块，定时更新 WiFi 信号和系统堆内存信息
└── appAudioRecord.cpp/.h # 音频录制模块，支持将 MIC 和 TTS 数据保存到 SD 卡
```

## 代码烧录流程

0. 确保已完成[快速开始](Quick_start.md)中开发环境的搭建。

1. 连接 T5AI 开发板到电脑，打开 Arduino IDE，选择 `TUYA_T5AI` 开发板，并选择正确的烧录端口。

> 注意：T5AI 系列开发板提供双串口通信，连接电脑后会检测到两个串口号，其中 UART0 用于固件烧录，请在 Arduino IDE 中选择正确的烧录口。

2. 在 Arduino IDE 中点击 `文件` -> `示例` -> `AI components` -> `YourChatBot`，打开示例代码。

3. 将示例文件中的授权码信息和产品 PID 替换为自己的信息。
    - [什么是授权码](https://tuyaopen.ai/zh/docs/quick-start#tuyaopen-%E4%B8%93%E7%94%A8%E6%8E%88%E6%9D%83%E7%A0%81)
    - [如何获取授权码](https://tuyaopen.ai/zh/docs/quick-start#tuyaopen-%E6%8E%88%E6%9D%83%E7%A0%81%E8%8E%B7%E5%8F%96)

```cpp
// 设备授权码（替换为自己的授权码）
#define TUYA_DEVICE_UUID    "uuidxxxxxxxxxxxxxxxx"
#define TUYA_DEVICE_AUTHKEY "keyxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"

```

4. （可选）如需启用音频录制功能（将 MIC 和 TTS 音频数据保存到 SD 卡），可将 `ENABLE_AUDIO_RECORDING` 设置为 `1`：

```cpp
#define ENABLE_AUDIO_RECORDING  1   // 默认为 0（关闭），设为 1 开启音频录制
```

5. 点击 Arduino IDE 左上角按钮烧录代码，终端出现以下信息说明烧录成功。

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

打开 Arduino IDE 自带的串口监视器（波特率 115200），在输入框中键入文本后按回车发送，即可通过串口与 AI Agent 进行文字对话。示例代码中通过 `TuyaAI.sendText()` 将串口接收到的文本发送到云端大模型，AI 的回复会在串口监视器和屏幕上同步显示。

#### 语音交互

语音交互支持四种对话模式，默认配置为 `唤醒对话` 模式。用户可通过**双击按键**在各模式间循环切换。

**唤醒词**：你好涂鸦（hey tuya）

| 模式编号 | 模式名称 | 说明 |
| :---: | :---: | --- |
| 0 | 长按对话（HOLD） | 用户需长按按键以启动语音输入，松开按键即结束语音输入 |
| 1 | 按键对话（ONE_SHOT） | 设备上电默认进入待机状态，用户短按按键后设备进入聆听状态，可进行单次语音对话 |
| 2 | 唤醒对话（WAKEUP） | 用户通过唤醒词将设备唤醒后，可进行单次对话，类似智能音箱交互 |
| 3 | 自由对话（FREE） | 用户通过唤醒词唤醒设备后可进行连续对话；若唤醒后 30 秒无对话，设备将再次进入待机状态 |

## 应用示例介绍

本示例作为完整功能展示，开发者可以根据需要从更为简洁的 [AI example](AI_API_Development.md) 熟悉 AI 开发流程。

### AI 开发

#### 初始化 AI 模块

```cpp
// 1. 配置并初始化 AI 核心
AIConfig_t aiConfig = {AI_CHAT_MODE_WAKEUP, 70, aiEventCallback, aiStateCallback, NULL};
TuyaAI.begin(aiConfig);

// 2. 初始化音频子系统
TuyaAI.Audio.begin();

// 3. 初始化 UI 子系统
appDisplayInit(UI_TYPE);
```

**参数说明：**

| 参数 | 说明 |
| --- | --- |
| `AI_CHAT_MODE_WAKEUP` | 初始对话模式，可选 `AI_CHAT_MODE_HOLD`(0) / `AI_CHAT_MODE_ONE_SHOT`(1) / `AI_CHAT_MODE_WAKEUP`(2) / `AI_CHAT_MODE_FREE`(3) |
| `70` | 初始音量（范围 0 ~ 100） |
| `aiEventCallback` | AI 事件回调函数，用于接收 ASR、TTS、文本流等所有 AI 事件数据 |
| `aiStateCallback` | AI 状态回调函数，用于接收设备状态变化通知 |
| `NULL` | 用户自定义数据指针，一般传 `NULL` |

`TuyaAI.Audio.begin()` 对设备的音频系统进行初始化，使能麦克风、扬声器以及音频的编解码功能。

UI 初始化通过 `appDisplay.h` 中的 `UI_TYPE` 宏确定使用哪种 UI 方案：

```c
#define UI_TYPE             BOT_UI_WECHAT   // 在 appDisplay.h 中修改
```

- `BOT_UI_WECHAT` : 内置 WeChat 风格 UI，开箱即用
- `BOT_UI_USER` : 仅初始化 LVGL，用户自行通过 LVGL 接口设计 UI 界面

#### AI 事件回调

```cpp
static void aiEventCallback(AI_USER_EVT_TYPE_E event, uint8_t *data, uint32_t len, void *arg);
```

AI 运行过程中会产生丰富的事件和数据，所有 AI 事件通过 `aiEventCallback` 统一分发处理。主要事件类型如下：

| 事件 | 说明 | 回调数据 |
| --- | --- | --- |
| `AI_USER_EVT_ASR_OK` | ASR 语音识别成功 | 识别出的文本内容 |
| `AI_USER_EVT_ASR_EMPTY` | ASR 结果为空（用户未说话） | 无 |
| `AI_USER_EVT_ASR_ERROR` | ASR 识别失败 | 无 |
| `AI_USER_EVT_MIC_DATA` | 麦克风原始 PCM 音频数据 | PCM 音频字节流 |
| `AI_USER_EVT_TTS_PRE` | TTS 即将开始播放 | 无 |
| `AI_USER_EVT_TTS_START` | TTS 开始播放 | 无 |
| `AI_USER_EVT_TTS_DATA` | TTS 音频数据块（MP3 格式） | MP3 音频字节流 |
| `AI_USER_EVT_TTS_STOP` | TTS 正常播放完成 | 无 |
| `AI_USER_EVT_TTS_ABORT` | TTS 被中断 | 无 |
| `AI_USER_EVT_TEXT_STREAM_START` | 大模型文本回复流开始 | 首段文本数据 |
| `AI_USER_EVT_TEXT_STREAM_DATA` | 大模型文本回复流数据 | 文本数据块 |
| `AI_USER_EVT_TEXT_STREAM_STOP` | 大模型文本回复流结束 | 无 |
| `AI_USER_EVT_SKILL` | Skill 技能数据（如音乐播放） | cJSON* 技能数据 |
| `AI_USER_EVT_EMOTION` | 文本标签中解析的情绪 | `AI_AGENT_EMO_T*` 情绪结构体 |
| `AI_USER_EVT_LLM_EMOTION` | 大模型推理出的用户情绪 | `AI_AGENT_EMO_T*` 情绪结构体 |
| `AI_USER_EVT_SERVER_VAD` | 服务端人声检测结束 | 无 |
| `AI_USER_EVT_MODE_SWITCH` | 对话模式切换 | 新模式编号（int） |
| `AI_USER_EVT_CHAT_BREAK` | 对话被中断 | 无 |

#### AI 状态回调

```cpp
static void aiStateCallback(AI_MODE_STATE_E state);
```

AI 设备在运行过程中会在不同状态间切换，`aiStateCallback` 负责接收状态变化通知，用户可据此更新 UI 或执行业务逻辑。状态列表如下：

| 状态 | 说明 |
| --- | --- |
| `AI_MODE_STATE_IDLE` | 待机状态 |
| `AI_MODE_STATE_INIT` | 初始化中 |
| `AI_MODE_STATE_LISTEN` | 聆听中（正在采集语音） |
| `AI_MODE_STATE_UPLOAD` | 上传中（语音数据上传云端） |
| `AI_MODE_STATE_THINK` | 思考中（等待大模型回复） |
| `AI_MODE_STATE_SPEAK` | 播报中（TTS 正在播放） |

### IoT 开发

#### IoT 初始化

```cpp
// 快速连按 3 次复位键可触发设备重新配网
TuyaIoT.resetNetcfg();
// 设置 IoT 事件回调函数
TuyaIoT.setEventCallback(tuyaIoTEventCallback);
// 设置设备授权码
TuyaIoT.setLicense(TUYA_DEVICE_UUID, TUYA_DEVICE_AUTHKEY);
// 启动 IoT 服务，传入产品 PID 和固件版本号
TuyaIoT.begin(TUYA_PRODUCT_ID, PROJECT_VERSION);
```

`TuyaIoT.resetNetcfg()` 使能"快速连按复位键重新配网"功能。当用户在短时间内连续按下 3 次复位键时，设备将清除已保存的网络配置并重新进入配网状态。

#### IoT 事件处理

```cpp
static void tuyaIoTEventCallback(tuya_event_msg_t *event);
```

所有 IoT 平台事件在 `tuyaIoTEventCallback` 中统一处理，主要事件包括：

| 事件 | 说明 |
| --- | --- |
| `TUYA_EVENT_BIND_START` | 设备开始配网，播放配网提示音 |
| `TUYA_EVENT_MQTT_CONNECTED` | MQTT 连接成功，此时可初始化 MCP 工具、上报设备状态 |
| `TUYA_EVENT_MQTT_DISCONNECT` | MQTT 连接断开 |
| `TUYA_EVENT_TIMESTAMP_SYNC` | 云端时间同步 |
| `TUYA_EVENT_RESET` | 设备被重置 |
| `TUYA_EVENT_UPGRADE_NOTIFY` | 收到 OTA 升级通知 |
| `TUYA_EVENT_DP_RECEIVE_OBJ` | 收到对象型 DP 数据（如音量控制） |
| `TUYA_EVENT_DP_RECEIVE_RAW` | 收到原始型 DP 数据 |

#### DP 数据交互

本示例演示了通过 DP（Data Point）实现云端音量控制功能：

```cpp
#define DPID_VOLUME  3  // 音量 DP ID

// 在 DP 接收回调中处理音量设置
case DPID_VOLUME:
    uint8_t volume = dp->value.dp_value;
    TuyaAI.setVolume(volume);
    break;
```

用户同样可以基于涂鸦云进行 IoT 开发，实现对 `powered by Tuya` 设备的控制。更多 DP 操作请参考 `00_IoT_SimpleExample` 示例。

### 音频开发

所有音频相关数据均通过 `aiEventCallback` 回调函数获取，通过 `AI_USER_EVT_TYPE_E` 事件类型区分：

| 事件 | 数据格式 | 说明 |
| --- | --- | --- |
| `AI_USER_EVT_ASR_OK` | UTF-8 文本 | 云端 ASR 语音识别结果 |
| `AI_USER_EVT_MIC_DATA` | PCM（16kHz, 16bit, mono） | 麦克风原始录音数据 |
| `AI_USER_EVT_TTS_DATA` | MP3 数据块 | 大模型下发的 TTS 音频数据流 |
| `AI_USER_EVT_TTS_PRE/START/STOP` | 无 | TTS 播放生命周期事件 |
| `AI_USER_EVT_TEXT_STREAM_START/DATA/STOP` | UTF-8 文本 | 大模型下发的文本数据流 |
| `AI_USER_EVT_SERVER_VAD` | 无 | 服务端人声检测（检测到用户停止说话） |

#### 音频录制到 SD 卡

在 `appAudioRecord.cpp/.h` 文件中演示了如何将 MIC 和 TTS 音频数据保存到 SD 卡。使用前需将 `ENABLE_AUDIO_RECORDING` 宏设置为 `1`。

- **MIC 录音**：以 `.pcm` 格式保存，采用内存缓冲区（默认 16KB）减少 SD 卡写入次数。在 `AI_MODE_STATE_LISTEN` 状态时自动开始录制，离开聆听状态时自动停止。
- **TTS 录音**：以 `.mp3` 格式保存，在 `AI_USER_EVT_TTS_START` 时开始，`AI_USER_EVT_TTS_STOP` 或 `AI_USER_EVT_TTS_ABORT` 时结束。
- 录音文件保存在 SD 卡的 `/ai_recordings` 目录下，按序号自动递增命名（如 `mic_001.pcm`、`tts_001.mp3`）。

### 按键交互

在 `appButton.cpp/.h` 文件中使用 `Button` 组件对按键功能进行了统一封装。默认使用 GPIO 12 引脚，低电平触发，内部上拉。

支持以下按键事件：

| 事件 | 行为 |
| --- | --- |
| 单击（`BUTTON_EVENT_SINGLE_CLICK`） | 委托给 `TuyaAI.modeKeyHandle()` 处理，触发当前模式下的对话行为 |
| 双击（`BUTTON_EVENT_DOUBLE_CLICK`） | 停止音频播放 → 中断当前对话 → 切换到下一个对话模式 → 播放模式提示音 |
| 长按（`BUTTON_EVENT_LONG_PRESS_START`） | 委托给 `TuyaAI.modeKeyHandle()` 处理（长按对话模式下启动录音） |
| 按下 / 抬起 | 委托给 `TuyaAI.modeKeyHandle()` 处理 |

双击切换模式的核心逻辑如下：

```cpp
if (event == BUTTON_EVENT_DOUBLE_CLICK) {
    TuyaAI.Audio.stop();           // 停止当前音频播放
    TuyaAI.interruptChat();        // 中断当前对话
    AI_CHAT_MODE_E nextMode = TuyaAI.nextChatMode();  // 获取下一个模式
    TuyaAI.saveModeConfig(nextMode, volume);           // 保存模式配置
    // 播放对应模式的提示音
}
```

> 更多 Button 组件使用方法请参考 `Peripheral/Button` 目录下的示例。

### UI 开发

在 `appDisplay.cpp/.h` 文件中对设备的 UI 功能进行了统一封装。通过修改 `appDisplay.h` 中的 `UI_TYPE` 宏即可切换 UI 方案。

#### BOT_UI_WECHAT

系统内置的 WeChat 风格 UI 页面，开箱即用。用户可通过 `TuyaAI.UI` 类的接口获取字体（`getTextFont`）、图标（`getIconFont` / `getWifiIcon`）、emoji 等 UI 资源，并控制内置 UI 组件的显示内容。

#### BOT_UI_USER

该参数仅启动 LVGL 图形引擎，用户可完全自主设计 UI。在 `appDisplay.cpp` 文件的 `_createUI()` 函数中提供了一个极简暗色主题的 UI 参考实现，布局如下：

```
┌──────────────────────────┐
│ [对话模式]      [WiFi]   │ ← 顶部状态栏 (30px)
├──────────────────────────┤
│       [ 设备状态 ]        │ ← 状态指示区 (36px)
├──────────────────────────┤
│ ┌────────────────────┐   │
│ │ 用户消息...         │◀──│ ← 用户气泡（右对齐，绿色）
│ └────────────────────┘   │
│   ┌────────────────────┐ │
│ ──▶ AI 回复...         │ │ ← AI 气泡（左对齐，灰色）
│   └────────────────────┘ │
└──────────────────────────┘
```

示例中还封装了以下显示接口，方便在回调中更新 UI：

| 接口 | 说明 |
| --- | --- |
| `appDisplaySetUserText()` | 显示用户文本 |
| `appDisplaySetAIText()` | 显示 AI 回复文本 |
| `appDisplayStreamStart/Append/End()` | 流式显示 AI 回复 |
| `appDisplaySetStatus()` | 更新设备状态文本 |
| `appDisplaySetMode()` | 更新对话模式显示 |
| `appDisplaySetWifi()` | 更新 WiFi 状态图标 |

#### Display 组件

用户可通过该组件直接驱动 TUYA-T5AI-BOARD 开发板自带的屏幕，该组件提供基础的屏幕绘制能力以及**摄像头数据直显**功能。

> 更多 Display 组件使用方法请参考 `Display` / `Camera` 目录下的示例。

### AI 高级功能

#### MCP

[什么是 MCP](https://tuyaopen.ai/zh/docs/cloud/tuya-cloud/ai-agent/mcp-management)：MCP（Model Context Protocol）是一种让 AI Agent 调用设备端本地工具的通用协议接口。

在 `appMCP.cpp/.h` 文件中演示了如何注册本地 MCP 工具供 AI Agent 调用。

> **注意**：MCP 工具必须在 MQTT 连接成功后注册。示例中通过订阅 `EVENT_MQTT_CONNECTED` 事件来确保注册时机正确。

**两种注册方式：**

1. **简单注册**（无参数工具）：使用 `TUYA_MCP_TOOL_ADD_SIMPLE` 宏

```cpp
// 注册无参数的工具
TUYA_MCP_TOOL_ADD_SIMPLE(
    "device_info_get",          // 工具名称
    "Get device information.",  // 工具描述（供 AI 理解用途）
    onGetDeviceInfo,            // 回调函数
    nullptr                     // 用户数据
);
```

2. **带参数注册**：使用 `TUYA_MCP_TOOL_REGISTER` 宏 + 属性定义

```cpp
// 定义工具参数
TuyaMCPPropDef volumeProps[] = {
    TuyaMCPPropIntRange("volume", "The volume level (0-100).", 0, 100)
};
// 注册带参数的工具
TUYA_MCP_TOOL_REGISTER(
    "device_audio_volume_set",  // 工具名称
    "Sets the device volume.",  // 工具描述
    onSetVolume,                // 回调函数
    nullptr,                    // 用户数据
    volumeProps, 1              // 参数定义和参数个数
);
```

**参数类型宏**：

| 宏 | 说明 |
| --- | --- |
| `TuyaMCPPropStr(name, desc)` | 字符串类型参数 |
| `TuyaMCPPropIntRange(name, desc, min, max)` | 整数类型参数（带范围限制） |
| `TuyaMCPPropIntDefRange(name, desc, def, min, max)` | 整数类型参数（带默认值和范围） |

**回调函数中的常用方法**：

| 方法 | 说明 |
| --- | --- |
| `TuyaAI.MCP.getPropertyInt(props, name, def)` | 获取整数类型参数值 |
| `TuyaAI.MCP.setReturnBool(retVal, value)` | 返回布尔值 |
| `TuyaAI.MCP.setReturnJson(retVal, json)` | 返回 JSON 对象 |
| `TuyaAI.MCP.setReturnImage(retVal, mime, data, len)` | 返回图片数据（如拍照结果） |

本示例提供了四种 MCP 工具：

| 工具名称 | 触发方式 | 功能说明 |
| --- | --- | --- |
| `device_info_get` | "获取设备信息" | 返回设备型号、序列号、固件版本等 JSON 信息 |
| `device_camera_take_photo` | "帮我拍照" | 调用摄像头拍照并将 JPEG 图片返回给 AI 进行内容识别 |
| `device_audio_volume_set` | "把音量调到 50" | 设置设备音量（0 ~ 100） |
| `device_audio_mode_set` | "切换到自由对话模式" | 切换对话模式（0=长按，1=按键，2=唤醒，3=自由） |

#### Skill

在 `aiEventCallback` 中通过 `AI_USER_EVT_SKILL` 事件解析 Skill 技能数据。使用 `TuyaAI.Skill.parse()` 方法进行解析，支持多种技能类型：

```cpp
case AI_USER_EVT_SKILL:
    SkillData_t skill;
    if (OPRT_OK == TuyaAI.Skill.parse(data, skill)) {
        switch (skill.type) {
            case SKILL_TYPE_MUSIC:          // 音乐播放
            case SKILL_TYPE_STORY:          // 故事播放
                // 获取音乐/故事的 URL、名称等信息
                // skill.music->src_array[0].url
                TuyaAI.Skill.dumpMusic(skill);   // 打印调试信息
                TuyaAI.Skill.freeMusic(skill);   // 释放资源（必须调用）
                break;
            case SKILL_TYPE_PLAY_CONTROL:   // 播放控制（上一曲/下一曲/暂停等）
                // skill.playControl.action
                break;
            case SKILL_TYPE_EMOTION:        // 情绪技能
                break;
        }
    }
    break;
```

#### Emotion

AI 聊天过程中的情绪识别包含两种来源：

- `AI_USER_EVT_EMOTION`：从文本标签中解析出的情绪信息
- `AI_USER_EVT_LLM_EMOTION`：由大模型推理得出的用户情绪

两者都通过 `AI_AGENT_EMO_T` 结构体返回，包含**情绪名称**（`name`）和**对应的 emoji**（`emoji`，Unicode 编码）。可使用 `TuyaAI.Skill.unicodeToUtf8()` 将 emoji 的 Unicode 编码转换为 UTF-8 字符串用于显示。

```cpp
case AI_USER_EVT_LLM_EMOTION:
    AI_AGENT_EMO_T *emo = (AI_AGENT_EMO_T *)data;
    Serial.print("情绪: "); Serial.println(emo->name);
    if (emo->emoji) {
        char utf8[8];
        TuyaAI.Skill.unicodeToUtf8(emo->emoji, utf8, sizeof(utf8));
        Serial.print("Emoji: "); Serial.println(utf8);
    }
    break;
```

### 状态监控

在 `appStatus.cpp/.h` 文件中实现了设备运行状态的周期性监控，包括：

- **WiFi 信号状态**：每 1 秒检测一次 WiFi 连接状态，状态变化时自动更新屏幕上的 WiFi 图标
- **堆内存监控**：每 5 秒输出一次空闲堆内存大小，便于开发调试
- **对话模式显示**：实时在屏幕上显示当前的对话模式名称


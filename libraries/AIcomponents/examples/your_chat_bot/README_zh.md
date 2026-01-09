# 你的聊天机器人

## 概述

这是一个功能完整的 AI 语音助手示例，结合了语音交互、显示集成和物联网连接。它展示了使用涂鸦 IoT 平台实现 AI 语音助手的完整功能，包括按钮控制、LED 指示灯、显示输出以及文本和语音交互功能。

## 功能特性

- **多模式交互**：支持语音（ASR）和文本输入
- **按钮控制**：物理按钮用于唤醒和对话控制
- **显示集成**：可选屏幕显示聊天消息和系统状态
- **音量控制**：通过云端命令调节音频音量
- **状态可视化**：实时状态和情绪显示
- **LED 指示灯**：设备状态的视觉反馈
- **网络配置**：自动配置并支持二维码
- **AI 音频流**：AI 响应的实时流式传输
- **唤醒词检测**：使用唤醒词免手动激活

## 硬件要求

- 涂鸦兼容的开发板（TUYA_T5AI）
- 连接到 GPIO 1 的 LED（用于状态指示）
- 连接到 GPIO 12 的按钮（用于用户交互）
- 显示模块（屏幕）
- 音频输出设备（扬声器）
- 音频输入设备（麦克风）
- 稳定的 Wi-Fi 连接（2.4G）
- 串口控制台（波特率 115200）

`TUYA_T5AI` 开发板推荐使用 [T5AI-Board 开发套件](https://tuyaopen.ai/zh/docs/hardware-specific/t5-ai-board/overview-t5-ai-board) 进行开发。

## 配置说明

### 设备凭证

上传代码前，需要配置以下参数：

```cpp
#define TUYA_DEVICE_UUID "uuidxxxxxxxxxxxxxxxx"
#define TUYA_DEVICE_AUTHKEY "keyxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
#define TUYA_PRODUCT_ID "alon7qgyjj8yus74"
```

您可以在 [涂鸦开发者平台](https://tuyaopen.ai/zh/docs/cloud/tuya-cloud/creating-new-product) 创建产品获得 PID 或者使用默认的 PID。

关于授权码的获取方式，参考 [授权码获取](https://tuyaopen.ai/zh/docs/quick-start#tuyaopen-%E6%8E%88%E6%9D%83%E7%A0%81%E8%8E%B7%E5%8F%96).
您必须替换自己的设备 UUID 和 AUTHKEY。

### 语言选择

设置音频提示的语言：

```cpp
#define LANG_CODE_ZH 0  // 0: 英语, 1: 中文
```

### 对话模式

```cpp
#define AI_AUDIO_WORK_MODE AI_AUDIO_WORK_ASR_WAKEUP_SINGLE_TALK // choice work mode：provide 4 modes to chat with AI Agent

#define AI_AUDIO_MODE_MANUAL_SINGLE_TALK     1
#define AI_AUDIO_WORK_VAD_FREE_TALK          2
#define AI_AUDIO_WORK_ASR_WAKEUP_SINGLE_TALK 3
#define AI_AUDIO_WORK_ASR_WAKEUP_FREE_TALK   4
```

### 硬件引脚

```cpp
#define LED_PIN 1           // 状态 LED
#define BUTTON_PIN0 12      // 交互按钮
```

### 数据点

```cpp
#define DPID_VOLUME 3       // 音量控制 DP ID
```

## 使用方法

### 初始设置

1. **上传代码**：修改授权码，将代码烧录到开发板
2. **打开串口监视器**：设置波特率为 115200
3. **等待初始化**：设备将显示系统信息
4. **设备配网**：使用[涂鸦 APP](https://tuyaopen.ai/zh/docs/quick-start/device-network-configuration) 配网

### 交互模式

#### 文本聊天（串口）

在串口监视器中输入消息与 AI 聊天：

```
🧍‍♂️ User: 今天天气怎么样？
AI: 今天 xxxxxxxxxx
================chat end=================
```

#### 语音交互（按钮）

##### 唤醒词

 - **你好涂鸦**
 - **Hey Tuya**

设备支持修改宏定义 `AI_AUDIO_WORK_MODE` 切换多种 [交互模式](https://tuyaopen.ai/zh/docs/applications/tuya.ai/ai-components/ai-audio-asr-impl)

#### LED 指示

- **熄灭**：待机状态
- **点亮**：监听/活动状态

## 核心功能

### 主要组件

- **`setup()`**：初始化所有硬件、IoT 连接、显示和 AI 组件
- **`loop()`**：处理持续的用户输入和状态管理
- **`handleUserInput()`**：处理来自串口监视器的文本输入
- **`buttonCallback()`**：处理语音交互的按钮按压事件
- **`tuyaIoTEventCallback()`**：管理 IoT 事件和网络状态
- **`tuyaAIEventCallback()`**：处理 AI 音频事件（ASR、文本、情绪）
- **`tuyaAIStateCallback()`**：更新系统状态（待机、监听、上传、播放）

### 按钮配置

```cpp
ButtonConfig_t cfg = {
  .debounceTime = 50,              // 按钮防抖时间（毫秒）
  .longPressTime = 2000,           // 长按阈值（毫秒）
  .longPressHoldTime = 500,        // 长按重复间隔（毫秒）
  .multiClickCount = 2,            // 多击检测的最大点击次数
  .multiClickInterval = 500        // 多击时间窗口（毫秒）
};
```

### AI 音频配置

```cpp
AI_AUDIO_CONFIG_T ai_audio_cfg = {
  .work_mode = AI_AUDIO_WORK_MODE,
  .evt_inform_cb = tuyaAIEventCallback,
  .state_inform_cb = tuyaAIStateCallback,
};
```

## AI 音频状态

系统在多个状态之间转换：

1. **STANDBY（待机）**：准备唤醒或手动激活
2. **LISTEN（监听）**：录制用户语音输入
3. **UPLOAD（上传）**：将音频发送到云端处理
4. **AI_SPEAK（播放）**：播放 AI 响应音频

## 显示集成

默认启用了 `ENABLE_CHAT_DISPLAY`，在 `app_display_init` 中启动了 LVGL 显示，用户可以在 `app_display.cpp` 中根据 LVGL API 创建自定义显示。

当前显示集成包含：

- **用户消息**：从用户语音转录的 ASR 文本
- **AI 消息**：来自 AI 的文本响应（流式或完整）
- **情绪**：AI 情绪的表情符号表示
- **状态**：当前系统状态（监听、播放、待机）
- **网络状态**：Wi-Fi 连接质量
- **音量级别**：当前音频音量

## 数据点处理

什么是 [DP点](https://tuyaopen.ai/zh/docs/applications)？

### 音量控制（DP ID 3）

从云端接收音量命令：
```cpp
case DPID_VOLUME: {
  uint8_t volume = dp->value.dp_value;
  TuyaAI.setVolume(volume);
  // 如果启用则更新显示
}
```

初始化后音量会自动上传到云端。

## 事件处理

### IoT 事件处理: tuyaIoTEventCallback()

- **TUYA_EVENT_RESET:** IOT 设备被重置
- **TUYA_EVENT_BIND_START:** 设备配对开始
- **TUYA_EVENT_BIND_TOKEN_ON:** 设备配对成功
- **TUYA_EVENT_ACTIVATE_SUCCESSED:** 设备激活成功
- **TUYA_EVENT_MQTT_CONNECTED:** 云连接建立
- **TUYA_EVENT_MQTT_DISCONNECT:** 云连接断开
- **TUYA_EVENT_DP_RECEIVE:** 接收到数据点命令
- **TUYA_EVENT_DP_RECEIVE_CJSON:** 接收到 JSON 格式数据点
- **TUYA_EVENT_DP_RECEIVE_OBJ:** 接收到对象格式数据点
- **TUYA_EVENT_DP_RECEIVE_RAW:** 接收到原始数据
- **TUYA_EVENT_UPGRADE_NOTIFY:** OTA 更新通知
- **TUYA_EVENT_RESET_COMPLETE:** 设备重置完成
- **TUYA_EVENT_TIMESTAMP_SYNC:** 时间戳同步
- **TUYA_EVENT_DPCACHE_NOTIFY:** 数据点缓存通知
- **TUYA_EVENT_BINDED_NOTIFY:** 绑定通知
- **TUYA_EVENT_DIRECT_MQTT_CONNECTED:** 直接 MQTT 连接成功

### AI 音频事件处理: tuyaAIEventCallback()

- **AI_AUDIO_EVT_NONE:** 无事件
- **AI_AUDIO_EVT_HUMAN_ASR_TEXT:** 语音转录文本
- **AI_AUDIO_EVT_AI_REPLIES_TEXT_START:** AI 响应文本开始
- **AI_AUDIO_EVT_AI_REPLIES_TEXT_DATA:** AI 响应文本数据
- **AI_AUDIO_EVT_AI_REPLIES_TEXT_END:** AI 响应文本结束
- **AI_AUDIO_EVT_AI_REPLIES_TEXT_INTERUPT:** AI 响应文本被中断
- **AI_AUDIO_EVT_AI_REPLIES_EMO:** AI 情绪
- **AI_AUDIO_EVT_ASR_WAKEUP:** ASR 唤醒

### AI 音频状态处理: tuyaAIStateCallback()
- **AI_AUDIO_STATE_STANDBY:** 待机状态
- **AI_AUDIO_STATE_LISTEN:** 监听状态
- **AI_AUDIO_STATE_UPLOAD:** 上传状态
- **AI_AUDIO_STATE_AI_SPEAK:** 播放状态

### 按键事件处理: buttonEventCallback()

- **BUTTON_EVENT_PRESS_DOWN:** 按键按下
- **BUTTON_EVENT_PRESS_UP:** 按键释放
- **BUTTON_EVENT_SINGLE_CLICK:** 单击
- **BUTTON_EVENT_DOUBLE_CLICK:** 双击
- **BUTTON_EVENT_MULTI_CLICK:** 多击
- **BUTTON_EVENT_LONG_PRESS_START:** 长按开始
- **BUTTON_EVENT_LONG_PRESS_HOLD:** 长按保持


## 音频提示: 

系统播放各种音频提示：
- **AI_AUDIO_ALERT_WAKEUP**：唤醒确认音
- **AI_AUDIO_ALERT_NETWORK_CFG**：网络配置模式
- **AI_AUDIO_ALERT_NETWORK_CONNECTED**：成功连接到云端
- **AI_AUDIO_ALERT_NOT_ACTIVE**：设备尚未激活

## 项目结构

```
your_chat_bot/
├── your_chat_bot.ino        # 主程序文件
├── app_config.h             # 应用配置
├── app_display.h            # 显示接口头文件
├── app_display.cpp          # 显示实现
├── app_system_info.h        # 系统信息头文件
├── app_system_info.cpp      # 系统信息实现
└── README.md                # 本文档
```


## 注意事项

- 设备需要活跃的互联网连接进行 AI 处理
- 启动期间按重置按钮 3 次可清除网络配置
- 音频处理在云端进行，需要稳定的网络连接
- 每个事件都会记录空闲堆内存以监控系统健康

## 相关文档

 - [TuyaOpen 官网](https://tuyaopen.ai)
 - [Github 代码库](https://github.com/tuya/TuyaOpen)
 - [Arduino-TuyaOpen 仓库](https://github.com/tuya/Arduino-TuyaOpen)
 - [设备配网](https://tuyaopen.ai/zh/docs/quick-start/device-network-configuration) (需下载 APP)

## 联系我们

您有任何好的建议和反馈，欢迎在 [Github](https://github.com/tuya/TuyaOpen/issues) 提交 issue ！！！

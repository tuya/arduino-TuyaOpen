# YourChatBot - AI 聊天机器人示例

## 概述

本示例演示如何使用 TuyaAI 库在涂鸦 T5AI 平台上构建一个完整的 AI 聊天机器人。它提供了一个可扩展的框架，包含 UI 显示、按钮交互、MCP 工具扩展和音频录制等功能模块。

## 唤醒词

- **你好涂鸦**
- **Hey Tuya**

## 功能特性

- **多种对话模式**：支持按键、唤醒词、自由对话等 4 种模式
- **可定制 UI**：支持自定义 LVGL UI 或使用内置 UI 模板
- **MCP 工具扩展**：支持注册自定义 MCP 工具（如拍照、获取设备信息）
- **事件驱动架构**：丰富的 AI 事件回调（ASR、TTS、情绪、技能等）
- **音频录制**：可选的 MIC/TTS 音频录制到 SD 卡
- **模块化设计**：按钮、显示、状态监控等独立模块

## 硬件要求

- 涂鸦 `TUYA_T5AI` 开发板
- 连接到 GPIO 1 的 LED（状态指示）
- 连接到 GPIO 12 的按钮（模式切换/语音触发）
- 连接到 GPIO 28 的扬声器
- 稳定的 Wi-Fi 连接
- 串口控制台（波特率 115200）

推荐使用 [T5AI-Board 开发套件](https://tuyaopen.ai/zh/docs/hardware-specific/t5-ai-board/overview-t5-ai-board) 进行开发。

## 项目结构

```
YourChatBot/
├── YourChatBot.ino      # 主程序入口
├── appDisplay.cpp/h     # UI 显示模块（支持自定义 LVGL）
├── appButton.cpp/h      # 按钮交互模块
├── appMCP.cpp/h         # MCP 工具注册模块
├── appStatus.cpp/h      # 状态监控模块（WiFi、内存等）
└── appAudioRecord.cpp/h # 音频录制模块（可选）
```

## 配置说明

上传代码前，需要配置以下参数：

```cpp
// 设备授权信息（必须替换为您自己的）
#define TUYA_DEVICE_UUID    "uuidxxxxxxxxxxxxxxxx"
#define TUYA_DEVICE_AUTHKEY "keyxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
#define TUYA_PRODUCT_ID     "9inb01mvjqh5zhhr"

// 硬件引脚配置
#define LED_PIN             1
#define BUTTON_PIN          12
#define SPK_PIN             28

// 功能开关
#define ENABLE_AUDIO_RECORDING  0   // 设为 1 启用音频录制
```

### 获取授权信息

- **PID（产品 ID）**：在 [涂鸦开发者平台](https://tuyaopen.ai/zh/docs/cloud/tuya-cloud/creating-new-product) 创建产品获取
- **UUID/AUTHKEY**：参考 [授权码获取指南](https://tuyaopen.ai/zh/docs/quick-start#tuyaopen-%E6%8E%88%E6%9D%83%E7%A0%81%E8%8E%B7%E5%8F%96)

### 对话模式

```cpp
AIConfig_t aiConfig = {
    .chatMode = AI_MODE_WAKEUP,  // 默认唤醒词模式
    .volume = 70,
    .eventCb = aiEventCallback,
    .stateCb = aiStateCallback,
    .userArg = NULL
};
```

可选模式：
| 模式 | 说明 |
|------|------|
| `AI_MODE_HOLD` | 按住按钮说话 |
| `AI_MODE_ONESHOT` | 按一下触发单次对话 |
| `AI_MODE_WAKEUP` | 唤醒词 + 单次对话 |
| `AI_MODE_FREE` | 自由对话模式 |

### UI 类型

在 `appDisplay.h` 中配置：

```cpp
#define UI_TYPE  BOT_UI_WECHAT  // 可选: BOT_UI_USER, BOT_UI_WECHAT, BOT_UI_CHATBOT
```

## 使用方法

1. **配置参数**：替换 UUID、AUTHKEY 和 PID
2. **上传代码**：使用 Arduino IDE 上传到 T5AI 开发板
3. **打开串口**：设置波特率 115200
4. **配网绑定**：使用涂鸦智能 APP 扫码绑定设备
5. **开始对话**：
   - 语音：说 "你好涂鸦" 或 "Hey Tuya"
   - 文本：在串口输入文字按回车

### 按钮操作

| 操作 | 功能 |
|------|------|
| 单击 | 触发语音输入（根据模式） |
| 双击 | 切换对话模式 |
| 长按 | 按住说话（HOLD 模式） |

## 核心组件

### TuyaAI 类架构

```cpp
TuyaAI              // 主控制器
├── TuyaAI.UI       // UI 显示管理
├── TuyaAI.Audio    // 音频输入输出
├── TuyaAI.MCP      // MCP 工具协议
└── TuyaAI.Skill    // 技能数据解析
```

### 事件回调

```cpp
void aiEventCallback(AIEvent_t event, uint8_t *data, uint32_t len, void *arg) {
    switch (event) {
        case AI_USER_EVT_ASR_OK:      // 语音识别成功
        case AI_USER_EVT_TTS_START:   // TTS 开始播放
        case AI_USER_EVT_EMOTION:     // 情绪识别
        case AI_USER_EVT_SKILL:       // 技能事件（音乐、故事等）
        // ... 更多事件
    }
}
```

### MCP 工具示例

在 `appMCP.cpp` 中注册自定义工具：

```cpp
// 获取设备信息工具
TuyaAI.MCP.registerTool(
    "get_device_info",
    "Get device information",
    onGetDeviceInfo, NULL,
    {}  // 无参数
);

// 拍照工具
TuyaAI.MCP.registerTool(
    "take_photo",
    "Take a photo with the camera",
    onTakePhoto, NULL,
    {TuyaMCPPropStrDef("quality", "Image quality", "medium")}
);
```

## 故障排除

| 问题 | 解决方案 |
|------|----------|
| 设备无法连接 | 检查 WiFi 凭证，确保网络可访问涂鸦云 |
| 无 AI 响应 | 验证设备已绑定，MQTT 已连接 |
| 串口无输出 | 确认波特率为 115200 |
| 唤醒词无反应 | 检查麦克风连接，降低环境噪音 |
| UI 不显示 | 检查 `UI_TYPE` 配置，确保 LVGL 已启用 |

## 注意事项

- 快速按复位键 3 次将清除网络配置
- GPIO 1 LED 指示设备运行状态
- 音频录制功能需要 SD 卡支持
- MCP 工具需要设备已连接 MQTT 后才能注册

## 相关文档

 - [TuyaOpen 官网](https://tuyaopen.ai)
 - [Github 代码库](https://github.com/tuya/TuyaOpen)
 - [Arduino-TuyaOpen 仓库](https://github.com/tuya/Arduino-TuyaOpen)
 - [设备配网](https://tuyaopen.ai/zh/docs/quick-start/device-network-configuration) (需下载 APP)

## 联系我们

欢迎在 [GitHub Issues](https://github.com/tuya/TuyaOpen/issues) 提交问题和建议！

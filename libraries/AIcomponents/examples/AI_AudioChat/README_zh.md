# AI 音频示例

## 概述

本示例演示如何使用涂鸦 IoT 平台将 AI 音频功能集成到物联网设备中。它通过串口输入提供与 AI 智能体的文本交互，允许用户与 AI 聊天并实时接收响应。

## 唤醒词

 - **你好涂鸦**
 - **Hey Tuya**

## 功能特性

- **文本输入接口**：通过串口监视器与 AI 智能体聊天
- **AI 音频集成**：利用 TuyaAI 库进行音频处理
- **物联网连接**：完全集成涂鸦 IoT 云平台
- **事件驱动架构**：动态处理音频状态和事件
- **网络配置**：支持设备绑定和配置重置
- **数据点处理**：处理对象和原始两种 DP 类型

## 硬件要求

- 涂鸦兼容的开发板（`TUYA_T5AI`）
- 连接到 GPIO 1 的 LED（可选，用于状态指示）
- 连接到 GPIO 12 的按钮（可选，用于重置功能）
- 稳定的 Wi-Fi 连接
- 串口控制台用于交互（波特率 115200）

`TUYA_T5AI` 开发板推荐使用 [T5AI-Board/T5AI-Core 开发套件](https://tuyaopen.ai/zh/docs/hardware-specific/t5-ai-board/overview-t5-ai-board) 进行开发。    

## 配置说明

上传代码前，需要配置以下参数：

```cpp
#define TUYA_DEVICE_UUID "uuidxxxxxxxxxxxxxxxx"
#define TUYA_DEVICE_AUTHKEY "keyxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
#define TUYA_PRODUCT_ID "9inb01mvjqh5zhhr"
```

您可以在 [涂鸦开发者平台](https://tuyaopen.ai/zh/docs/cloud/tuya-cloud/creating-new-product) 创建产品获得 PID 或者使用默认的 PID。

关于授权码的获取方式，参考 [授权码获取](https://tuyaopen.ai/zh/docs/quick-start#tuyaopen-%E6%8E%88%E6%9D%83%E7%A0%81%E8%8E%B7%E5%8F%96).
您必须替换自己的设备 UUID 和 AUTHKEY。


## 使用方法

1. **上传代码**：将代码上传到开发板
2. **打开串口监视器**：设置波特率为 115200
3. **设备初始化**：等待设备连接到 Wi-Fi 和涂鸦云
4. **开始聊天**：在串口监视器中输入消息并按回车
5. **AI 响应**：AI 将实时回复您的查询

### 串口监视器交互示例

```
🧍‍♂️ User: 你好，最近怎么样？
AI: 我很好！有什么可以帮助您的吗？
================chat end=================
```

## 核心功能

### 主要组件

- **`handleUserInput()`**：处理文本输入并将其发送到 AI 智能体
- **`tuyaIoTEventCallback()`**：管理 IoT 事件（绑定、MQTT 连接、DP 接收等）
- **`aiEventCallback()`**：处理 AI 音频事件（ASR 文本、AI 回复、情绪、唤醒）
- **`aiStateCallback()`**：跟踪 AI 音频状态转换

### AI 音频工作模式

本示例使用 `WAKE_UP` 模式，结合了：
- 自动语音识别（ASR）
- 唤醒词检测
- 单轮对话处理

### 事件处理

代码响应各种事件：
- **设备绑定**：处理与涂鸦应用的初始配对
- **MQTT 连接**：管理云连接状态
- **数据点**：处理来自云端或应用的命令
- **AI 文本流**：实时接收和显示 AI 响应
- **情绪**：处理 AI 响应中的情绪数据

## 故障排除

- **设备无法连接**：检查 Wi-Fi 凭证并确保网络访问正常
- **无 AI 响应**：验证设备已绑定到涂鸦云且 MQTT 已连接
- **串口输入无效**：确保串口监视器设置为 115200 波特率，并使用换行符终止

## 注意事项

- 如果重置按钮被按下三次，设备将自动重置网络配置
- GPIO 1 上的 LED 指示设备状态
- 本示例需要有效的涂鸦 IoT 账户和正确配置的产品


## 相关文档

 - [TuyaOpen 官网](https://tuyaopen.ai)
 - [Github 代码库](https://github.com/tuya/TuyaOpen)
 - [Arduino-TuyaOpen 仓库](https://github.com/tuya/Arduino-TuyaOpen)
 - [设备配网](https://tuyaopen.ai/zh/docs/quick-start/device-network-configuration) (需下载 APP)

## 联系我们

您有任何好的建议和反馈，欢迎在 [Github](https://github.com/tuya/TuyaOpen/issues) 提交 issue ！！！

# AI MCP 工具调用

本示例演示如何使用 Arduino-TuyaOpen 框架中的 **MCP（Model Context Protocol）** 功能，注册自定义工具供 AI Agent 调用。用户通过语音或文本与 AI 对话时，AI 可自主判断并调用设备端注册的工具，实现对硬件的智能控制。

> 本示例仅支持 TUYA_T5AI 平台。推荐使用 [T5AI-Board 开发套件](https://tuyaopen.ai/zh/docs/hardware-specific/t5-ai-board/overview-t5-ai-board) 进行开发。

## 工作原理

1. 设备端通过 MCP 协议向云端注册工具（包含工具名称、描述、参数定义）
2. 用户通过语音或文本向 AI 发出请求（如"把音量调到 50"）
3. AI 大模型根据用户意图自动匹配并调用对应的设备端工具
4. 设备端执行工具回调函数，完成操作后将结果返回给 AI
5. AI 将执行结果以自然语言回复给用户

## 代码烧录流程

0. 确保已完成[快速开始](Quick_start.md)中开发环境的搭建。

1. 连接 T5AI 开发板到电脑，打开 Arduino IDE，选择 `TUYA_T5AI` 开发板，并选择正确的烧录端口。

> 注意：T5AI 系列开发板提供双串口通信，连接电脑后会检测到两个串口号，其中 UART0 用于固件烧录，请在 Arduino IDE 中选择正确的烧录口。

2. 在 Arduino IDE 中点击 `文件` -> `示例` -> `AI components` -> `04_AI_Mcp`，打开示例代码。

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

配网成功后，可通过语音或串口文本与 AI 进行对话。以下为本示例注册的 MCP 工具及其触发方式：

| 语音/文本指令 | 触发的 MCP 工具 | 功能说明 |
| --- | --- | --- |
| "把音量调到 50" | `device_audio_volume_set` | 设置设备音量（0 ~ 100） |

> 用户可以自然语言表达需求，AI 会自动理解并调用对应的工具。

## 示例代码说明

### MCP 初始化

MCP 工具必须在 **MQTT 连接成功后** 注册。本示例通过订阅 `EVENT_MQTT_CONNECTED` 事件来确保注册时机正确：

```cpp
// 在 setup() 中订阅 MQTT 连接事件，触发一次性回调
tal_event_subscribe(EVENT_MQTT_CONNECTED, "mcpRegisterTool", initMCPTools, SUBSCRIBE_TYPE_ONETIME);
```

在回调函数 `initMCPTools` 中完成 MCP 的初始化和工具注册：

```cpp
static int initMCPTools(void* data) {
    // 1. 初始化 MCP 服务
    if (OPRT_OK != TuyaAI.MCP.begin()) {
        PR_ERR("MCP init failed");
        return -1;
    }

    // 2. 定义工具参数
    TuyaMCPPropDef volumeProps[] = {
        TuyaMCPPropIntRange("volume", "The volume level to set (0-100).", 0, 100)
    };

    // 3. 注册带参数的工具
    TUYA_MCP_TOOL_REGISTER(
        "device_audio_volume_set",                    // 工具名称
        "Sets the device's volume level.\n"           // 工具描述（供 AI 理解用途）
        "Parameters:\n"
        "- volume (int): The volume level (0-100).\n"
        "Response:\n"
        "- Returns true if set successfully.",
        onSetVolume,                                  // 回调函数
        nullptr,                                      // 用户数据
        volumeProps, 1                                // 参数定义和参数个数
    );
    return OPRT_OK;
}
```

### MCP 工具回调

当 AI 调用注册的工具时，对应的回调函数会被执行：

```cpp
static int onSetVolume(MCPPropertyList_t props, MCPReturnValue_t ret, void *userData) {
    // 从参数列表中获取 volume 值（默认值 50）
    int volume = TuyaAI.MCP.getPropertyInt(props, "volume", 50);
    // 执行音量设置
    TuyaAI.Audio.setVolume(volume);
    // 返回执行结果
    TuyaAI.MCP.setReturnBool(ret, true);
    return 0;
}
```

### MCP 注册方式

MCP 提供两种注册方式：

**1. 带参数注册**（使用 `TUYA_MCP_TOOL_REGISTER` 宏）：

```cpp
TuyaMCPPropDef volumeProps[] = {
    TuyaMCPPropIntRange("volume", "The volume level (0-100).", 0, 100)
};
TUYA_MCP_TOOL_REGISTER("tool_name", "description", callback, userData, volumeProps, 1);
```

**2. 简单注册**（无参数工具，使用 `TUYA_MCP_TOOL_ADD_SIMPLE` 宏）：

```cpp
TUYA_MCP_TOOL_ADD_SIMPLE("tool_name", "description", callback, userData);
```

### 参数类型宏

| 宏 | 说明 |
| --- | --- |
| `TuyaMCPPropStr(name, desc)` | 字符串类型参数 |
| `TuyaMCPPropIntRange(name, desc, min, max)` | 整数类型参数（带范围限制） |
| `TuyaMCPPropIntDefRange(name, desc, def, min, max)` | 整数类型参数（带默认值和范围） |

### 回调函数中的常用方法

| 方法 | 说明 |
| --- | --- |
| `TuyaAI.MCP.getPropertyInt(props, name, def)` | 获取整数类型参数值 |
| `TuyaAI.MCP.getPropertyString(props, name, def)` | 获取字符串类型参数值 |
| `TuyaAI.MCP.getPropertyBool(props, name, def)` | 获取布尔类型参数值 |
| `TuyaAI.MCP.setReturnBool(ret, value)` | 返回布尔值 |
| `TuyaAI.MCP.setReturnJson(ret, json)` | 返回 JSON 对象 |
| `TuyaAI.MCP.setReturnImage(ret, mime, data, len)` | 返回图片数据 |

### AI 事件回调

本示例处理以下 AI 事件：

| 事件 | 说明 |
| --- | --- |
| `AI_USER_EVT_ASR_OK` | ASR 语音识别成功，在串口显示用户所说内容 |
| `AI_USER_EVT_TEXT_STREAM_START` | AI 文本回复开始（包含 MCP 工具调用结果） |
| `AI_USER_EVT_TEXT_STREAM_DATA` | AI 文本回复数据 |
| `AI_USER_EVT_TEXT_STREAM_STOP` | AI 文本回复结束 |

> 当 AI 调用 MCP 工具后，工具的返回结果会作为上下文传递给大模型，大模型再将结果以自然语言通过 TEXT_STREAM 事件回复给用户。

## 相关文档

- [什么是 MCP](https://tuyaopen.ai/zh/docs/cloud/tuya-cloud/ai-agent/mcp-management)
- [TuyaOpen 官网](https://tuyaopen.ai)
- [Github 代码库](https://github.com/tuya/TuyaOpen)

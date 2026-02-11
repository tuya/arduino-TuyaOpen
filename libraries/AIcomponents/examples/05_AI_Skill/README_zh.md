# AI Skill 技能解析

本示例演示如何使用 Arduino-TuyaOpen 框架中的 **Skill 技能** 功能，解析并处理 AI 下发的技能事件。包括音乐/故事播放、播放控制、情绪识别，以及将 TTS 语音数据保存到 SD 卡等功能。

> 本示例仅支持 TUYA_T5AI 平台。推荐使用 [T5AI-Board 开发套件](https://tuyaopen.ai/zh/docs/hardware-specific/t5-ai-board/overview-t5-ai-board) 进行开发。

### 项目文件结构

```
05_AI_Skill/
├── 05_AI_Skill.ino        # 主程序入口，AI 事件回调中分发 Skill/Emotion/TTS 事件
└── skill_handler.cpp/.h   # 技能处理模块，封装技能解析、情绪处理和 TTS 录制
```

## 代码烧录流程

0. 确保已完成[快速开始](Quick_start.md)中开发环境的搭建。

1. 连接 T5AI 开发板到电脑，打开 Arduino IDE，选择 `TUYA_T5AI` 开发板，并选择正确的烧录端口。

> 注意：T5AI 系列开发板提供双串口通信，连接电脑后会检测到两个串口号，其中 UART0 用于固件烧录，请在 Arduino IDE 中选择正确的烧录口。

2. 在 Arduino IDE 中点击 `文件` -> `示例` -> `AI components` -> `05_AI_Skill`，打开示例代码。

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

配网成功后，可通过语音或串口文本与 AI 进行对话。以下语音指令可触发不同技能：

| 语音指令示例 | 触发的技能类型 | 说明 |
| --- | --- | --- |
| "播放音乐" / "放一首歌" | 音乐技能（MUSIC） | 返回音乐播放 URL 和元数据 |
| "讲个故事" | 故事技能（STORY） | 返回故事播放 URL 和元数据 |
| "下一首" / "暂停" | 播放控制（PLAY_CONTROL） | 控制播放行为 |
| 任意对话 | 情绪识别（EMOTION） | AI 推理当前对话情绪 |

## 示例代码说明

### 技能处理模块初始化

在 `setup()` 中调用 `skillInit()` 初始化技能处理模块，分配 TTS 数据缓冲区（128KB）：

```cpp
skillInit();
```

### AI 事件回调

本示例在 `aiEventCallback` 中处理以下事件：

| 事件 | 说明 | 处理逻辑 |
| --- | --- | --- |
| `AI_USER_EVT_ASR_OK` | ASR 语音识别成功 | 在串口显示用户所说内容 |
| `AI_USER_EVT_TTS_DATA` | TTS 音频数据块 | 收集到缓冲区 `skillCollectTts()` |
| `AI_USER_EVT_TTS_STOP` | TTS 播放结束 | 保存缓冲区到 SD 卡 `skillSaveTts()` |
| `AI_USER_EVT_TEXT_STREAM_DATA` | 文本流数据 | 在串口输出 |
| `AI_USER_EVT_SKILL` | Skill 技能数据 | 解析技能 `skillHandleEvent()` |
| `AI_USER_EVT_EMOTION` | 文本标签中的情绪 | 处理情绪 `skillHandleEmotion()` |
| `AI_USER_EVT_LLM_EMOTION` | 大模型推理的情绪 | 处理情绪 `skillHandleEmotion()` |
| `AI_USER_EVT_PLAY_CTL_PLAY` | 播放控制：播放 | 串口输出 |
| `AI_USER_EVT_PLAY_CTL_PAUSE` | 播放控制：暂停 | 串口输出 |
| `AI_USER_EVT_PLAY_CTL_NEXT` | 播放控制：下一首 | 串口输出 |
| `AI_USER_EVT_PLAY_CTL_PREV` | 播放控制：上一首 | 串口输出 |

### Skill 技能解析

使用 `TuyaAI.Skill.parse()` 解析技能数据，支持多种技能类型：

```cpp
void skillHandleEvent(void *data) {
    SkillData_t skill;
    if (OPRT_OK == TuyaAI.Skill.parse(data, skill)) {
        switch (skill.type) {
            case SKILL_TYPE_MUSIC:
            case SKILL_TYPE_STORY:
                // 获取音乐/故事资源信息
                // skill.music->action       - 播放动作
                // skill.music->src_cnt      - 资源数量
                // skill.music->src_array[i].url  - 播放 URL
                TuyaAI.Skill.dumpMusic(skill);   // 打印调试信息
                TuyaAI.Skill.freeMusic(skill);   // 释放资源（必须调用）
                break;

            case SKILL_TYPE_PLAY_CONTROL:
                // skill.playControl.action  - 播放控制动作
                break;

            case SKILL_TYPE_EMOTION:
                // 情绪技能
                break;
        }
    }
}
```

**技能类型表：**

| 类型 | 宏 | 说明 |
| --- | --- | --- |
| 音乐 | `SKILL_TYPE_MUSIC` | 音乐播放请求，包含播放 URL 列表 |
| 故事 | `SKILL_TYPE_STORY` | 故事播放请求，包含播放 URL 列表 |
| 播放控制 | `SKILL_TYPE_PLAY_CONTROL` | 播放/暂停/上一首/下一首 |
| 情绪 | `SKILL_TYPE_EMOTION` | 情绪类型技能 |

> **重要**：`SKILL_TYPE_MUSIC` 和 `SKILL_TYPE_STORY` 类型的技能数据包含动态分配的资源，处理完毕后必须调用 `TuyaAI.Skill.freeMusic(skill)` 释放内存。

### 情绪识别

AI 聊天过程中的情绪识别包含两种来源：

- `AI_USER_EVT_EMOTION`：从文本标签中解析出的情绪信息
- `AI_USER_EVT_LLM_EMOTION`：由大模型推理得出的用户情绪

两者都通过 `AI_AGENT_EMO_T` 结构体返回，包含**情绪名称**（`name`）和**对应的 emoji**（`emoji`，Unicode 编码）：

```cpp
void skillHandleEmotion(AI_AGENT_EMO_T *emo) {
    if (!emo) return;
    Serial.print("[Emotion] ");
    Serial.print(emo->name ? emo->name : "unknown");

    if (emo->emoji) {
        char utf8[8];
        if (TuyaAI.Skill.unicodeToUtf8(emo->emoji, utf8, sizeof(utf8)) > 0) {
            Serial.print(" ");
            Serial.print(utf8);
        }
    }
    Serial.println();
}
```

> 可使用 `TuyaAI.Skill.unicodeToUtf8()` 将 emoji 的 Unicode 编码转换为 UTF-8 字符串用于显示。

### TTS 音频保存

本示例在每次对话后将 AI 的 TTS 语音回复保存到 SD 卡：

1. `skillCollectTts()`：在 `AI_USER_EVT_TTS_DATA` 事件中收集 TTS 数据到内存缓冲区（128KB）
2. `skillSaveTts()`：在 `AI_USER_EVT_TTS_STOP` 事件中将缓冲区数据写入 SD 卡

文件保存路径为 `/sdcard/tts_output.mp3`，每次新的 TTS 会覆盖上一次的文件。

## 相关文档

- [TuyaOpen 官网](https://tuyaopen.ai)
- [Github 代码库](https://github.com/tuya/TuyaOpen)

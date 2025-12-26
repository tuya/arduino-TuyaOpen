# Audio Recorder 示例

## 概述

这个示例演示了如何使用重新封装的 C++ Audio 类，通过 P12 按键实现录音和播放功能。

## 主要特性

### Audio 类的核心功能

1. **初始化和清理**
   - `Audio::Audio()` - 构造函数
   - `begin(config)` - 初始化音频设备
   - `end()` - 关闭音频设备并释放资源

2. **录音控制**
   - `startRecord()` - 开始录音
   - `stopRecord()` - 停止录音
   - `getRecordedDataLen()` - 获取已录制数据大小
   - `clearRecordedData()` - 清除录制的数据

3. **播放控制**
   - `playback()` - 播放已录制的音频
   - `stopPlayback()` - 停止播放

4. **音量控制**
   - `setVolume(volume)` - 设置音量 (0-100)
   - `getVolume()` - 获取当前音量

5. **状态管理**
   - `getStatus()` - 获取当前状态
   - `setStatus(status)` - 设置状态

## 配置结构体

```cpp
typedef struct {
    uint32_t duration_ms;    // 最大可录制时长（毫秒）
    uint8_t volume;          // 音量（0-100）
    uint32_t sample_rate;    // 采样率
} AUDIO_CONFIG_T;
```

## 使用方式

### 硬件要求
- P12 按键（按下时为 LOW，松开时为 HIGH）
- 音频输入设备（麦克风）
- 音频输出设备（扬声器）

### 按键操作

**按下 P12 按键**
- 开始录音
- LED 或日志显示录音进度

**松开 P12 按键**
- 停止录音
- 自动播放录制的音频

### 工作流程

```
启动 → 初始化音频 → 等待按键
                    ↓
                 按下 P12
                    ↓
                 开始录音 → 显示进度
                    ↓
                 松开 P12
                    ↓
                 停止录音 → 自动播放
                    ↓
                 播放完成 → 等待下次按键
```

## 配置示例

```cpp
// 音频配置
Audio::AUDIO_CONFIG_T audio_config = {
    .duration_ms = 5000,   // 最多录制 5 秒
    .volume = 70,          // 70% 音量
    .sample_rate = 16000   // 16kHz 采样率
};

audio.begin(&audio_config);
```

## 状态流程

```
IDLE → RECORDING → END → PLAYING → IDLE
```

### 状态说明

- **RECORDER_STATUS_IDLE** - 空闲状态，等待按键
- **RECORDER_STATUS_RECORDING** - 正在录音，等待按键释放
- **RECORDER_STATUS_END** - 录音已结束
- **RECORDER_STATUS_PLAYING** - 正在播放

## 改进点

相比于原始的 C 风格实现，新的 C++ 接口提供了：

1. **更好的封装** - 所有状态和资源都封装在类中
2. **自动资源管理** - 通过构造函数和析构函数管理资源
3. **更清晰的 API** - 面向对象的方法调用
4. **错误处理** - 返回 OPERATE_RET 来表示操作结果
5. **灵活配置** - 通过结构体进行灵活的配置
6. **数据访问** - 提供了多种方式访问录制的数据

## 注意事项

- 确保 PSRAM 足够大以容纳录制的音频数据
- 录制时长应根据硬件能力和内存大小合理设置
- 播放前确保有足够的已录制数据
- P12 按键应配置为 INPUT_PULLUP 模式
- 按键按下时为 LOW，松开时为 HIGH

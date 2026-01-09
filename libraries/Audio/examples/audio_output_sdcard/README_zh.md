# 音频输出到SD卡示例

## 概述
本示例演示如何使用按键控制录音,并将录音保存到SD卡的PCM和WAV两种格式。具有自动文件编号和实时数据流写入功能,防止内存溢出。

## 功能特性
- **按键控制录音**: 按下开始,松开停止
- **双文件格式输出**: 同时保存PCM(原始)和WAV(带头)文件
- **自动递增文件名**: 文件按顺序编号(recording_001, recording_002等)
- **实时文件写入**: 录音时直接将音频数据流式写入SD卡
- **状态机设计**: 稳健的录音生命周期管理
- **LED指示**: 录音期间的视觉反馈

## 硬件要求
- 涂鸦T5AI开发板(必需)
- 连接到音频输入的麦克风
- 已插入并格式化的SD卡
- 连接到GPIO 12的按键
- 连接到GPIO 1的LED(可选,用于视觉反馈)

## 引脚配置
- **按键**: GPIO 12(带内部上拉)
- **LED**: GPIO 1
- **扬声器使能**: GPIO 28(自动管理)

## 按键控制
- **按下**: 开始录音
- **松开**: 停止录音并保存文件到SD卡

## 文件输出
所有录音保存在SD卡的`/recordings`目录中:
- `recording_001.pcm` - 原始PCM音频数据
- `recording_001.wav` - 带WAV头的PCM数据
- `recording_002.pcm`, `recording_002.wav` - 下一个录音
- ... 依此类推

## 使用说明

### 1. 准备SD卡
- 将SD卡格式化为FAT32
- 在上电前插入SD卡

### 2. 上传程序
- 在Arduino IDE中打开`audio_output_sdcard.ino`
- 选择TUYA_T5AI作为开发板
- 上传程序

### 3. 操作
1. **开机** - 串口监视器会显示初始化状态
2. **按下按键** - LED亮起,开始录音
3. **松开按键** - LED熄灭,文件保存到SD卡
4. **查看串口监视器** 获取录音统计和文件信息
5. **重复操作** 创建更多带自动递增编号的录音

### 4. 访问录音
- 从开发板取出SD卡
- 插入电脑访问`/recordings`文件夹
- 使用任何音频播放器播放WAV文件
- PCM文件可以用音频编辑软件处理

## 音频配置
示例使用以下音频设置:
- **采样率**: 16kHz
- **位深度**: 16位
- **声道**: 单声道
- **最大录音时长**: 30秒
- **文件命名**: `recording_XXX.pcm` / `recording_XXX.wav`

## 技术细节

### 状态机
录音过程遵循以下状态:
1. **IDLE**: 等待按键按下
2. **RECORD_START**: 打开PCM文件
3. **RECORDING**: 持续将音频数据写入文件
4. **RECORD_END**: 停止录音,创建WAV文件

### 实时文件写入
与将所有音频缓冲到内存不同,本示例在录音期间直接将数据流式传输到SD卡:
```cpp
void savePCMFromRingBuffer() {
    // 持续将环形缓冲区排空到文件
    // 防止长时间录音的内存溢出
}
```

### WAV文件生成
录音完成后,处理PCM文件以创建WAV文件:
1. 读取PCM文件获取数据大小
2. 使用音频参数生成WAV头
3. 将头部+PCM数据写入WAV文件

### 文件编号
启动时,系统扫描`/recordings`目录并找到最大的现有编号,然后为下一个录音递增。

## 串口监视器输出示例
```
========================================
Audio Recorder to SD Card
Platform board:      TUYA_T5AI
========================================
Initializing SD card...
SD card mounted successfully
Created recordings directory
Next recording number: 1
Ready to record!

*** Start Recording ***
Recording...
Recording: 1s, 32000 bytes
Recording: 2s, 64000 bytes
Recording: 3s, 96000 bytes

*** Stop Recording ***
Recording stopped. Duration: 3045 ms, Total: 97440 bytes

=== Creating WAV file ===
PCM file size: 97440 bytes
WAV file created, data size: 97440 bytes
✓ Files saved successfully
Saved as: recording_001.pcm/.wav
Ready for next recording
```

## 故障排除

**SD卡未检测到:**
- 检查SD卡是否正确插入
- 验证SD卡格式化为FAT32
- 尝试不同的SD卡

**没有录制音频:**
- 检查麦克风连接
- 验证按键连接到GPIO 12
- 查看串口监视器的错误消息

**未创建文件:**
- SD卡可能已满 - 检查可用空间
- SD卡可能被写保护
- 查看串口监视器的"Write error"消息

**音频质量问题:**
- 检查采样率设置是否符合要求
- 验证麦克风工作正常
- 尝试在更安静的环境中录音

## 代码结构
- `audio_output_sdcard.ino` - 主程序文件
- `wav_encode.h` / `wav_encode.cpp` - WAV头生成工具
- `README.md` - 英文文档
- `README_zh.md` - 本文件

## 依赖库
- **Audio库**: 涂鸦音频录音和播放
- **Button库**: 防抖按键事件处理
- **FS库**: 文件系统操作(SD卡)
- **Log库**: 串口日志工具

## 注意事项
- 仅支持TUYA_T5AI平台
- 每次录音最长30秒(可配置)
- 录音编号可达999后回绕
- 文件保存期间LED闪烁

## 高级用法

### 自定义录音时长
```cpp
#define MAX_RECORD_DURATION_MS 60000  // 60秒
```

### 更改音频质量
```cpp
#define AUDIO_SAMPLE_RATE 8000   // 较低质量,较小文件
#define AUDIO_SAMPLE_RATE 16000  // 默认
#define AUDIO_SAMPLE_RATE 48000  // 较高质量,较大文件
```

### 自定义文件路径
```cpp
#define RECORDINGS_DIR "/my_recordings"
```

## 相关示例
- `audio_recorder` - 无SD卡的录音和播放
- `audio_speaker` - 从闪存或SD卡播放MP3文件
- `SDCardDemo` - 基本SD卡文件操作


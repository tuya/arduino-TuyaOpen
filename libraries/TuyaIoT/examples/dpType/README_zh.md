# TuyaIoT 数据点类型示例

## 概述

本示例演示了如何在涂鸦物联网应用中处理不同的数据点（DP）类型。展示了如何读写各种 DP 类型，包括布尔型、数值型、枚举型、字符串型、位图型和原始数据类型。

## 功能特性

- 处理多种 DP 类型（布尔、数值、枚举、字符串、位图、原始数据）
- 从涂鸦云端读取 DP 值
- 向涂鸦云端写入 DP 值
- 按钮控制设备重置
- 网络配置管理

## 硬件要求

- 涂鸦支持的开发板（ESP32、T2、T3、T5 等）
- 连接到 BUTTON_BUILTIN 引脚的按钮

## 数据点定义

示例定义了以下数据点：

- `DPID_SWITCH` (20): 布尔型 - 开关状态
- `DPID_MODE` (21): 枚举型 - 工作模式
- `DPID_BRIGHT` (22): 数值型 - 亮度等级
- `DPID_BITMAP` (101): 位图型 - 多个标志位
- `DPID_STRING` (102): 字符串型 - 文本数据
- `DPID_RAW` (103): 原始型 - 二进制数据

## 配置

上传前，需要配置以下参数：

```cpp
#define TUYA_DEVICE_UUID "uuidxxxxxxxxxxxxxxxx"      // 替换为你的设备 UUID
#define TUYA_DEVICE_AUTHKEY "xxxxxxxxxxxxxxxxxxxxxxxx" // 替换为你的授权密钥
```

产品 ID: `2avicuxv6zgeiquf`（在代码中配置）

## 工作原理

1. **初始化**: 初始化串口通信和日志系统
2. **许可证设置**: 读取开发板许可证或使用硬编码凭据
3. **物联网连接**: 连接到涂鸦物联网平台
4. **DP 处理**: 接收来自云端的 DP 命令并报告状态
5. **按钮控制**: 长按按钮移除设备

## DP 操作

### 布尔型（开关）
```cpp
bool switchStatus = 0;
TuyaIoT.read(event, DPID_SWITCH, switchStatus);
TuyaIoT.write(DPID_SWITCH, switchStatus);
```

### 数值型（整数）
```cpp
int brightValue = 0;
TuyaIoT.read(event, DPID_BRIGHT, brightValue);
TuyaIoT.write(DPID_BRIGHT, brightValue);
```

### 枚举型（模式）
```cpp
uint32_t mode = 0;
TuyaIoT.read(event, DPID_MODE, mode);
TuyaIoT.write(DPID_MODE, mode);
```

### 字符串型
```cpp
char *strValue = NULL;
TuyaIoT.read(event, DPID_STRING, strValue);
TuyaIoT.write(DPID_STRING, strValue);
```

### 原始型（二进制）
```cpp
uint8_t *rawValue = NULL;
uint16_t len = 0;
TuyaIoT.read(event, DPID_RAW, rawValue, len);
TuyaIoT.write(DPID_RAW, rawValue, len);
```

## 使用方法

1. 将固件烧录到设备
2. 打开串口监视器，波特率 115200
3. 设备将连接到涂鸦物联网平台
4. 使用涂鸦智能 App 控制数据点
5. 监视串口输出查看 DP 值
6. 长按按钮（3秒）移除设备

## 按钮控制

- **短按**: 无动作（可自定义）
- **长按（3秒）**: 从涂鸦物联网平台移除设备

## 处理的事件

- `TUYA_EVENT_BIND_START`: 设备绑定开始
- `TUYA_EVENT_ACTIVATE_SUCCESSED`: 设备激活成功
- `TUYA_EVENT_TIMESTAMP_SYNC`: 时间同步
- `TUYA_EVENT_DP_RECEIVE_OBJ`: 接收对象型 DP 数据
- `TUYA_EVENT_DP_RECEIVE_RAW`: 接收原始型 DP 数据

## 依赖库

- TuyaIoT 库
- Log 库

## 注意事项

- 确保拥有有效的涂鸦设备许可证
- 为你的设备类型配置正确的产品 ID
- 示例演示了所有常见的 DP 类型
- DP ID 必须与涂鸦物联网平台上的产品模型匹配

## 故障排除

- 如果设备无法连接，验证 UUID 和 AuthKey
- 检查产品 ID 是否与涂鸦平台上的产品匹配
- 确保 DP ID 与产品模型匹配
- 查看串口输出获取详细日志

## 了解更多

- 了解如何在涂鸦物联网平台定义数据点
- 学习不同 DP 类型及其使用场景
- 了解如何处理双向 DP 通信

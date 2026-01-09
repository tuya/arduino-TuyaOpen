# 天气获取示例

## 概述

本示例演示了如何将涂鸦物联网天气服务集成到你的设备中。展示了如何从涂鸦云端获取天气数据，并通过 LED 指示灯控制智能开关。

## 功能特性

- 从涂鸦物联网天气服务获取天气数据
- 带 LED 控制的智能开关功能
- 基于按钮的用户交互
- LED 状态指示（关闭、开启、闪烁）
- 网络配置管理
- 通过 Ticker 进行内存监控

## 硬件要求

- 涂鸦支持的开发板
- 连接到 LED_BUILTIN 引脚的 LED
- 连接到 BUTTON_BUILTIN 引脚的按钮

## 配置

上传前需要配置以下参数：

```cpp
#define TUYA_DEVICE_UUID "uuidxxxxxxxxxxxxxxxx"      // 你的设备 UUID
#define TUYA_DEVICE_AUTHKEY "xxxxxxxxxxxxxxxxxxxxxxxx" // 你的授权密钥
```

产品 ID: `qhivvyqawogv04e4`

## 引脚配置

- `LED_BUILTIN`: 状态 LED（低电平点亮）
- `BUTTON_BUILTIN`: 用户按钮（低电平有效）

## 工作原理

1. **初始化**: 设置日志、LED、按钮和物联网连接
2. **天气服务**: 初始化 TuyaIoTWeather 服务
3. **LED 控制**: 根据开关 DP 管理 LED 状态
4. **按钮处理**: 短按切换开关，长按移除设备
5. **天气演示**: 演示天气数据获取
6. **堆监控**: 每5秒报告空闲堆内存

## 数据点

- `DPID_SWITCH` (1): 布尔型 - 开关状态（控制 LED）

## 按钮控制

- **短按**: 切换开关/LED 状态
- **长按（3秒）**: 从涂鸦物联网平台移除设备

## LED 状态

- **关闭**: 开关为关闭状态
- **开启**: 开关为开启状态
- **闪烁（500ms）**: 设备处于绑定模式

## 使用方法

1. 将固件烧录到设备
2. 打开串口监视器，波特率 115200
3. 设备连接到涂鸦物联网平台
4. 使用按钮控制 LED 或使用涂鸦智能 App
5. 天气数据将自动获取

## 天气服务

示例包含 `weatherGetDemo()` 函数演示天气数据获取。你可以自定义此函数来：
- 获取当前天气
- 获取天气预报
- 获取温度、湿度等
- 显示天气信息

## 处理的事件

- `TUYA_EVENT_BIND_START`: 设备进入绑定模式
- `TUYA_EVENT_ACTIVATE_SUCCESSED`: 设备激活完成
- `TUYA_EVENT_MQTT_CONNECTED`: 连接到涂鸦云端
- `TUYA_EVENT_TIMESTAMP_SYNC`: 时间同步
- `TUYA_EVENT_DP_RECEIVE_OBJ`: 接收 DP 命令

## 依赖库

- TuyaIoT 库
- TuyaIoTWeather 库
- tLed 库
- Log 库
- Ticker 库

## 注意事项

- 确保拥有有效的涂鸦设备许可证
- 天气服务需要设备在线
- 空闲堆监控有助于跟踪内存使用
- tLed 类提供了带闪烁功能的简单 LED 控制

## 故障排除

- 如果连接失败，检查 UUID 和 AuthKey
- 验证网络配置
- 监视串口输出查看错误信息
- 如果设备行为异常，检查空闲堆内存

## 高级功能

- 自定义天气数据获取
- 添加更多 DP 进行额外控制
- 实现基于天气的自动化
- 存储天气历史记录

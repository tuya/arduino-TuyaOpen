# 按钮示例

## 概述

本示例演示了如何使用 Button 类处理涂鸦物联网设备上的各种按钮事件。展示了正确的按钮初始化、消抖和多个按钮的事件处理。

## 功能特性

- 支持多个按钮
- 按钮事件检测（按下、释放、单击）
- 可配置的消抖时间
- 长按检测
- 多击检测
- 上拉/下拉配置
- 事件回调系统

## 硬件要求

- 涂鸦支持的开发板
- 两个按钮（或修改为一个按钮）：
  - 按钮 0: GPIO 12
  - 按钮 1: GPIO 13
- 按钮可以接成低电平有效（带上拉）或高电平有效（带下拉）

## 引脚配置

- `BUTTON_PIN0`: GPIO 12 - 第一个按钮
- `BUTTON_PIN1`: GPIO 13 - 第二个按钮

## 按钮事件

可以检测以下事件：

- `BUTTON_EVENT_PRESS_DOWN`: 按钮被按下
- `BUTTON_EVENT_PRESS_UP`: 按钮被释放
- `BUTTON_EVENT_SINGLE_CLICK`: 检测到单击
- `BUTTON_EVENT_DOUBLE_CLICK`: 检测到双击（可配置）
- `BUTTON_EVENT_LONG_PRESS_START`: 检测到长按
- `BUTTON_EVENT_LONG_PRESS_HOLD`: 长按期间保持
- ···

## 配置参数

```cpp
ButtonConfig_t cfg;
cfg.debounceTime = 50;          // 消抖时间（毫秒）
cfg.longPressTime = 2000;       // 长按阈值（毫秒）
cfg.longPressHoldTime = 500;    // 保持重复间隔（毫秒）
cfg.multiClickCount = 2;        // 多击的点击次数
cfg.multiClickInterval = 500;   // 点击之间的最大间隔（毫秒）
```

## 引脚配置

```cpp
PinConfig_t pinCfg;
pinCfg.pin = BUTTON_PIN0;                // GPIO 引脚号
pinCfg.level = TUYA_GPIO_LEVEL_LOW;      // 有效电平（LOW 或 HIGH）
pinCfg.pullMode = TUYA_GPIO_PULLUP;      // 上拉或下拉
```

## 工作原理

1. **初始化**: 配置按钮参数和 GPIO 引脚
2. **事件注册**: 为所需事件注册回调函数
3. **自动检测**: Button 类处理消抖和事件检测
4. **回调执行**: 当事件发生时调用注册的函数

## 使用方法

1. 将按钮连接到指定的 GPIO 引脚
2. 将固件烧录到设备
3. 打开串口监视器，波特率 115200
4. 按下按钮查看串口输出的事件

## 示例输出

```
[Button0] Event: 0  // 按下
Button Down

[Button0] Event: 1  // 释放
Button UP

[Button0] Event: 2  // 单击
Button Single Click
```

## 自定义

### 添加更多事件

```cpp
Button0.setEventCallback(BUTTON_EVENT_DOUBLE_CLICK, buttonCallback);
Button0.setEventCallback(BUTTON_EVENT_LONG_PRESS_START, buttonCallback);
```

### 高电平有效配置

```cpp
pinCfg.level = TUYA_GPIO_LEVEL_HIGH;
pinCfg.pullMode = TUYA_GPIO_PULLDOWN;
```

### 调整时间参数

```cpp
cfg.debounceTime = 30;        // 更快的消抖
cfg.longPressTime = 1000;     // 更短的长按时间
```

## 事件回调

```cpp
void buttonCallback(char *name, ButtonEvent_t event, void *arg)
{
    // name: 按钮标识符（"Button0"、"Button1"）
    // event: 按钮事件类型
    // arg: 可选的用户数据
}
```

## 依赖库

- Button 库（Peripherals）
- Log 库

## 常见用例

- 单按钮：电源开/关
- 双击：模式切换
- 长按：恢复出厂设置
- 多个按钮：导航、控制
- 保持：连续调节

## 注意事项

- 消抖消除机械抖动造成的误触发
- 长按时间应 > 消抖时间
- 多击间隔决定点击速度容差
- 每个按钮可以有不同的配置
- Button 类使用非阻塞检测

## 故障排除

- 如果事件不触发：检查接线和上拉配置
- 如果多个事件触发：增加消抖时间
- 如果长按太敏感：增加 longPressTime
- 查看串口输出获取调试信息

## 最佳实践

- 为你的按钮使用适当的消抖时间（通常 20-100ms）
- 根据用户体验设置长按时间（1-3秒）
- 只注册需要的事件以节省资源
- 使用实际硬件测试按钮响应

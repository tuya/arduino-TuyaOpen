# Ticker 闪烁示例

## 概述

本示例演示了如何使用 Ticker 库创建周期性和一次性定时器回调。展示了使用非阻塞定时器以不同速率闪烁 LED。

## 功能特性

- 周期性定时器回调
- 一次性定时器回调
- 非阻塞操作
- 多个独立定时器
- 动态定时器重新配置
- LED 控制演示

## 硬件要求

- 涂鸦支持的开发板
- 内置 LED（LED_BUILTIN）

## 工作原理

1. **闪烁定时器**: 每 0.1 秒切换 LED（10 Hz）
2. **改变定时器**: 10 秒后，将闪烁速率改为 0.5 秒（2 Hz）
3. **非阻塞**: 所有计时在后台进行

## Ticker 方法

### 周期性回调

```cpp
ticker.attach(seconds, callback);
```
每隔 `seconds` 秒调用一次 `callback`。

### 一次性回调

```cpp
ticker.once(seconds, callback);
```
`seconds` 秒后调用一次 `callback`。

### 停止定时器

```cpp
ticker.detach();
```
停止定时器。

## 代码说明

### 初始设置
```cpp
blinker.attach(0.1, blinkCallback);  // 每 0.1 秒闪烁
changer.once(10, changeCallback);     // 10 秒后改变
```

### 回调函数

**blinkCallback()**: 切换 LED 状态
```cpp
void blinkCallback() {
  ledStatus = !ledStatus;
  digitalWrite(LED_BUILTIN, ledStatus);
}
```

**changeCallback()**: 改变闪烁速率
```cpp
void changeCallback() {
  blinkerPace = 0.5;               // 新速率：0.5 秒
  blinker.detach();                 // 停止旧定时器
  blinker.attach(0.5, blinkCallback); // 以新速率启动
}
```

## 使用方法

1. 将固件烧录到设备
2. LED 将快速闪烁（每秒 10 次）
3. 10 秒后，LED 将缓慢闪烁（每秒 2 次）
4. 改变自动发生

## 时序特性

| 阶段 | 速率 | 频率 | 持续时间 |
|-------|------|-----------|----------|
| 初始 | 0.1 秒 | 10 Hz | 10 秒 |
| 改变后 | 0.5 秒 | 2 Hz | 无限期 |

## 常见用例

- LED 状态指示
- 周期性传感器读取
- 心跳信号
- 超时检测
- 自动保存功能
- 周期性网络请求

## 重要注意事项

- **非阻塞**: 不使用 delay()
- **后台执行**: 回调在中断上下文中运行
- **保持回调简短**: 不要在回调中执行长时间操作
- **多个定时器**: 可以创建多个 Ticker 实例
- **精度**: 时序通常准确但不保证实时

## 回调最佳实践

✅ **推荐**:
- 切换 GPIO
- 设置标志
- 更新计数器
- 简单的状态改变

❌ **避免**:
- Serial.print()（谨慎使用）
- 延迟函数
- 长时间计算
- 内存分配
- 阻塞操作

## 高级示例

### 多个 LED
```cpp
Ticker led1Ticker;
Ticker led2Ticker;

led1Ticker.attach(0.5, toggleLed1);
led2Ticker.attach(0.3, toggleLed2);
```

### 超时检测
```cpp
Ticker timeout;

void startOperation() {
  timeout.once(5.0, handleTimeout);
  // 执行操作
}

void operationComplete() {
  timeout.detach();  // 取消超时
}
```

### 周期性数据采集
```cpp
Ticker dataTicker;

void setup() {
  dataTicker.attach(60, collectData);  // 每分钟
}

void collectData() {
  // 读取传感器，记录数据等
}
```

## 故障排除

- **LED 不闪烁**: 检查 LED_BUILTIN 定义和接线
- **时序不准确**: 回调可能受其他中断影响
- **系统不稳定**: 检查回调是否太长或太复杂
- **内存问题**: 避免在回调中动态分配内存

## 依赖库

- Ticker 库（已包含）

## 注意事项

- Ticker 内部使用硬件定时器
- 时序分辨率取决于系统节拍率
- 回调在中断上下文中执行
- 注意共享变量的并发访问
- 对 ISR 和主代码之间共享的变量使用 volatile 关键字

## 相关示例

- TickerParameter: 展示如何向回调传递参数
- 查看 Arduino 定时器文档了解更多定时器选项

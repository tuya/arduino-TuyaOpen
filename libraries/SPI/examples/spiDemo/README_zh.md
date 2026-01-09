# SPI 演示示例

## 概述

本示例演示了涂鸦开发板上的基本 SPI（串行外设接口）通信。展示了如何使用 SPI 库初始化 SPI 并传输数据。

## 功能特性

- SPI 总线初始化
- 使用 SPI 进行数据传输
- 可配置的 SPI 设置（速度、位顺序、模式）
- 基于事务的通信

## 硬件要求

- 涂鸦支持的开发板
- SPI 外设设备（可选，用于测试）

## 引脚配置

- **SCK（时钟）**: GPIO 14
- **MOSI（主机输出从机输入）**: GPIO 16
- **MISO（主机输入从机输出）**: GPIO 17
- **CS（片选）**: GPIO 15

**注意**: 引脚分配可能因开发板而异。请检查你的开发板引脚定义。

## SPI 设置

```cpp
SPISettings(8000000, MSBFIRST, SPI_MODE0)
```

- **时钟速度**: 8 MHz（T2 推荐范围：1-8 MHz）
- **位顺序**: MSBFIRST（最高位优先）- T2 仅支持 MSB 优先
- **SPI 模式**: SPI_MODE0（CPOL=0，CPHA=0）

## SPI 模式

| 模式 | CPOL | CPHA | 描述 |
|------|------|------|-------------|
| MODE0 | 0 | 0 | 时钟空闲为低，前沿采样 |
| MODE1 | 0 | 1 | 时钟空闲为低，后沿采样 |
| MODE2 | 1 | 0 | 时钟空闲为高，前沿采样 |
| MODE3 | 1 | 1 | 时钟空闲为高，后沿采样 |

## 工作原理

1. **初始化**: `SPI.begin()` 初始化 SPI 总线
2. **开始事务**: `beginTransaction()` 设置 SPI 参数
3. **数据传输**: `transfer()` 向 SPI 设备发送数据
4. **结束事务**: `endTransaction()` 释放 SPI 总线
5. **重复**: 每 3 秒传输一次数据

## 使用方法

1. 将 SPI 设备连接到指定引脚
2. 将固件烧录到设备
3. 设备将通过 SPI 持续发送 "Hello Arduino."
4. 使用逻辑分析仪或示波器验证 SPI 信号

## 代码示例

```cpp
SPI.begin();

SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
SPI.transfer(data, length);
SPI.endTransaction();
```

## 传输方法

### 单字节传输
```cpp
uint8_t response = SPI.transfer(data);
```

### 缓冲区传输
```cpp
SPI.transfer(buffer, bufferSize);
```

### 双向传输
```cpp
SPI.transfer(txBuffer, rxBuffer, length);
```

## 接线示例

```
开发板（T2）       SPI 设备
---------          ----------
GPIO 14 (SCK)  --> SCK
GPIO 16 (MOSI) --> MOSI/SDI
GPIO 17 (MISO) <-- MISO/SDO
GPIO 15 (CS)   --> CS/SS
GND            --> GND
3.3V           --> VCC
```

## 重要注意事项

- **T2 时钟范围**: 推荐 1-8 MHz
- **T2 位顺序**: 仅支持 MSBFIRST
- **电压电平**: 确保 SPI 设备兼容 3.3V
- **CS（片选）**: 如需要需手动控制
- **上拉电阻**: 某些设备可能需要上拉电阻

## 常见 SPI 设备

- SD 卡
- LCD/OLED 显示屏
- 传感器（加速度计、陀螺仪）
- ADC/DAC 转换器
- 闪存芯片
- 无线模块

## 故障排除

- **无通信**: 检查接线和引脚分配
- **数据乱码**: 验证时钟速度和 SPI 模式
- **间歇性错误**: 检查 CS 时序和信号完整性
- **设备无响应**: 验证电源和地线连接

## 性能提示

- 对大数据传输使用 DMA（如果支持）
- 对高速通信最小化事务开销
- 保持线缆长度短以确保信号完整性
- 为你的设备使用适当的时钟速度

## 依赖库

- SPI 库（已包含）

## 了解更多

- 学习 SPI 协议规范
- 使用逻辑分析仪调试 SPI 通信
- 查看 SPI 设备数据手册获取正确设置
- 尝试不同的时钟速度和模式

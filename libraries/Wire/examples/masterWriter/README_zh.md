# I2C 主机写入示例

## 概述

本示例演示了如何使用 Wire 库作为 I2C 主机向 I2C 从机设备发送数据。通过传输递增的字节值展示基本的 I2C 通信。

## 功能特性

- I2C 主机模式操作
- 向从机设备传输数据
- 自动字节递增
- 简单的周期性通信

## 硬件要求

- 涂鸦支持的开发板
- 地址为 0x08 的 I2C 从机设备
- SDA 和 SCL 线上的上拉电阻（通常为 4.7kΩ）

## I2C 引脚配置

默认 I2C 引脚（查看你的开发板规格）：
- **SDA（数据）**: 通常由开发板定义
- **SCL（时钟）**: 通常由开发板定义

## 工作原理

1. **初始化**: `Wire.begin()` 初始化 I2C 主机模式
2. **传输**:
   - 开始向设备 #8（地址 0x08）传输
   - 发送一个字节值
   - 结束传输
3. **递增**: 每次传输值增加 1
4. **重复**: 每 500ms 发送新值

## 代码说明

### 设置
```cpp
void setup() {
  Serial.begin(115200);
  Wire.begin();  // 作为主机加入 I2C 总线（不需要地址）
}
```

### 循环
```cpp
Wire.beginTransmission(8);  // 开始向设备 #8 传输
Wire.write(x);              // 发送一个字节
Wire.endTransmission();     // 停止传输
```

## I2C 通信流程

```
主机                      从机（地址：8）
  |                              |
  |------ 起始条件 ------------>|
  |------ 从机地址（8）-------->|
  |<--------- ACK --------------|
  |------ 数据字节（x）-------->|
  |<--------- ACK --------------|
  |------ 停止条件 ------------>|
  |                              |
```

## 使用方法

1. 将 I2C 从机设备连接到 I2C 引脚
2. 确保上拉电阻已连接
3. 将从机设备地址设为 0x08（或修改代码）
4. 将固件烧录到设备
5. 打开串口监视器，波特率 115200
6. 观察 "Sending: X" 消息

## 示例输出

```
Sending: 0
Sending: 1
Sending: 2
Sending: 3
...
Sending: 255
Sending: 0
Sending: 1
...
```

## Wire 库方法

### 主机模式

```cpp
Wire.begin();                        // 初始化为主机
Wire.beginTransmission(address);     // 开始传输
Wire.write(data);                    // 写入数据字节
Wire.write(buffer, length);          // 写入缓冲区
Wire.endTransmission();              // 结束传输
Wire.requestFrom(address, quantity); // 从从机请求数据
Wire.available();                    // 检查可用字节数
Wire.read();                         // 读取一个字节
```

## 接线示例

```
主机设备              I2C 从机（地址：8）
-----------           ---------------------
SDA ----------+-------- SDA
              |
           4.7kΩ（上拉到 3.3V）
              |
SCL ----------+-------- SCL
              |
           4.7kΩ（上拉到 3.3V）
              |
GND -------------------- GND
3.3V ------------------- VCC
```

## I2C 地址

- 有效范围：0x08 至 0x77（十进制 8 至 119）
- 保留地址：
  - 0x00 至 0x07：保留
  - 0x78 至 0x7F：保留

## 常见 I2C 设备

- 传感器：温度、压力、加速度计等
- EEPROM：存储器
- RTC：实时时钟
- DAC/ADC：模拟转换
- 带 I2C 转接板的 LCD 显示屏
- GPIO 扩展器

## 故障排除

### 无通信
- 检查接线连接
- 验证上拉电阻是否存在
- 确认从机设备地址
- 检查从机电源供应

### 数据错误
- 如果接线较长，降低 I2C 时钟速度
- 使用示波器检查信号完整性
- 确保地线正确连接
- 验证电压电平匹配（3.3V 或 5V）

### 时钟延展
- 某些从机使用时钟延展
- 确保主机支持它
- 可能需要调整超时设置

## 高级功能

### 多个从机
```cpp
Wire.beginTransmission(8);
Wire.write(data1);
Wire.endTransmission();

Wire.beginTransmission(9);
Wire.write(data2);
Wire.endTransmission();
```

### 从从机读取
```cpp
Wire.requestFrom(8, 2);  // 从设备 #8 请求 2 字节
while (Wire.available()) {
  char c = Wire.read();
  Serial.print(c);
}
```

### 时钟速度
```cpp
Wire.setClock(100000);  // 100kHz（标准模式）
Wire.setClock(400000);  // 400kHz（快速模式）
```

## 性能考虑

- **标准模式**: 100 kHz（默认）
- **快速模式**: 400 kHz
- **线缆长度**: 保持短以确保可靠通信
- **上拉电阻**: 典型值 4.7kΩ，根据电容调整

## 依赖库

- Wire 库（已包含）

## 注意事项

- 主机模式在 begin() 中不需要地址参数
- 始终使用 endTransmission() 完成传输
- 检查 endTransmission() 的返回值以查看错误：
  - 0: 成功
  - 1: 数据太长
  - 2: 地址上收到 NACK
  - 3: 数据上收到 NACK
  - 4: 其他错误

## 相关示例

- Master Reader: 从从机接收数据
- Slave Sender/Receiver: 实现 I2C 从机

# WiFi 扫描双天线示例

## 概述

本示例演示如何使用双天线配置执行 WiFi 网络扫描。它展示了如何配置和使用两个天线以改善 WiFi 接收和信号分集。双天线设置可以自动或手动选择最佳天线进行发送和接收，提高整体 WiFi 性能和可靠性。

## 功能特性

- **双天线配置**：配置两个 GPIO 引脚用于天线切换
- **自动天线选择**：让系统选择最佳天线
- **TX/RX 独立控制**：发送和接收的独立天线选择
- **网络扫描**：以改进的接收扫描可用 WiFi 网络
- **分集支持**：硬件天线分集以获得更好的信号质量

## 硬件要求

- 支持双天线的 Tuya Open 开发板
- 两个 WiFi 天线（或天线连接器）
- 用于编程和串口监视的 USB 数据线

**注意：** 并非所有开发板都支持双天线配置。使用本示例前请验证你的开发板功能。

## 配置说明

### 天线 GPIO 引脚

配置连接到天线开关的 GPIO 引脚：

```cpp
#define GPIO_ANT1 2   // 天线 1 的 GPIO
#define GPIO_ANT2 25  // 天线 2 的 GPIO
```

**重要：** 这些引脚编号必须与你的硬件设计匹配。错误的引脚可能会损坏开发板或天线。

### 天线选择模式

示例对 TX 和 RX 都使用自动天线选择：

```cpp
WiFi.setDualAntennaConfig(GPIO_ANT1, GPIO_ANT2, 
                          WIFI_RX_ANT_AUTO,  // 自动选择 RX 天线
                          WIFI_TX_ANT_AUTO); // 自动选择 TX 天线
```

## 工作原理

### 双天线配置

`setDualAntennaConfig()` 函数设置天线分集：

```cpp
bool err = WiFi.setDualAntennaConfig(
    GPIO_ANT1,          // 天线 1 的 GPIO
    GPIO_ANT2,          // 天线 2 的 GPIO
    WIFI_RX_ANT_AUTO,   // RX 天线模式
    WIFI_TX_ANT_AUTO    // TX 天线模式
);
```

### 天线选择模式

RX 和 TX 天线的可用模式：

- **WIFI_RX_ANT_AUTO** / **WIFI_TX_ANT_AUTO**：基于信号质量的自动选择
- **WIFI_RX_ANT0** / **WIFI_TX_ANT0**：强制使用天线 0（ANT1）
- **WIFI_RX_ANT1** / **WIFI_TX_ANT1**：强制使用天线 1（ANT2）

### 天线分集

天线分集通过以下方式提高性能：

1. **信号比较**：持续监控两个天线的信号质量
2. **自动切换**：切换到信号更好的天线
3. **多径减少**：减少信号反射的干扰
4. **覆盖改善**：在不同方向上更好的覆盖

### 扫描过程

1. 将 WiFi 设置为站点模式
2. 配置双天线设置
3. 断开任何网络
4. 扫描可用网络
5. 显示带有 RSSI 的结果
6. 每 5 秒重复扫描

## 使用方法

1. **验证硬件**：确保你的开发板支持双天线
2. **配置 GPIO**：更新 GPIO 引脚编号以匹配你的硬件
3. **上传程序**：上传到你的 Tuya Open 开发板
4. **打开串口监视器**：设置波特率为 115200
5. **观察结果**：查看天线配置状态和扫描结果

### 预期输出

**配置成功：**
```
Dual Antenna configuration successfully done!
Setup done
scan start
scan done
5 networks found
1: HomeNetwork (-42)*
2: OfficeWiFi (-58)*
3: GuestNetwork (-65) 
4: PublicHotspot (-73) 
5: Neighbor_2.4G (-85)*

scan start
...
```

**配置失败：**
```
Dual Antenna configuration failed!
Setup done
...
```

## 天线选择策略

### 自动模式（推荐）

```cpp
WiFi.setDualAntennaConfig(GPIO_ANT1, GPIO_ANT2, 
                          WIFI_RX_ANT_AUTO, 
                          WIFI_TX_ANT_AUTO);
```

**优点：**
- 最佳整体性能
- 适应变化的条件
- 无需手动干预

**适用于：**
- 移动或旋转设备
- 可变信号环境
- 通用应用

### 固定主天线

```cpp
WiFi.setDualAntennaConfig(GPIO_ANT1, GPIO_ANT2, 
                          WIFI_RX_ANT0,  // RX 使用 ANT1
                          WIFI_TX_ANT0); // TX 使用 ANT1
```

**优点：**
- 一致的行为
- 较低功耗（无切换）
- 可预测的性能

**适用于：**
- 已知最佳天线位置
- 固定安装
- 功耗关键应用

### 分离 TX/RX 配置

```cpp
WiFi.setDualAntennaConfig(GPIO_ANT1, GPIO_ANT2, 
                          WIFI_RX_ANT_AUTO,  // RX 自动
                          WIFI_TX_ANT0);     // TX 固定
```

**适用于：**
- 不同的天线特性
- 针对特定用例优化
- 一个天线具有更好的 TX 性能

## 应用场景

- **移动设备**：方向变化的机器人、无人机、车辆
- **恶劣环境**：具有多径干扰的工业环境
- **扩展范围**：在大空间中最大化覆盖
- **定向天线**：在全向和定向之间切换
- **冗余**：主天线故障时的备用天线
- **信号优化**：始终使用最佳可用天线

## 故障排除

### "Dual Antenna configuration failed!"

- 开发板可能不支持双天线功能
- GPIO 引脚可能不正确
- 引脚可能已被其他外设使用
- 检查开发板文档以了解支持的引脚

### 信号无改善

- 天线可能太近
- 两个天线在相同方向
- 分集增益不足
- 天线未正确连接

### 频繁切换天线

- 在变化的环境中正常
- 可能表示相似的信号强度
- 可能略微增加功耗
- 如有问题，考虑固定天线

### 一个天线不工作

- 检查 GPIO 引脚配置
- 验证物理天线连接
- 使用固定天线模式测试以隔离问题
- 检查硬件损坏

### 性能降低

- GPIO 引脚可能与其他功能冲突
- 某些情况下的切换开销
- 尝试固定天线模式进行比较

## 硬件考虑

### 天线放置

- **间隔**：天线间隔至少 λ/4（在 2.4 GHz 时约 3 厘米）
- **方向**：不同方向最大化分集
- **隔离**：最小化天线之间的耦合
- **间距**：使天线远离金属物体

### 天线类型

- **全向**：在所有方向上的均等辐射（常见）
- **定向**：在一个方向上的聚焦波束（更高增益）
- **PCB 天线**：集成在板上
- **外部**：通过 U.FL/IPEX 连接器连接

### GPIO 要求

- 必须是专用 GPIO 引脚
- 在 WiFi 操作期间不能与其他功能共享
- 检查开发板原理图以了解天线开关电路
- 典型实现使用由 GPIO 控制的 RF 开关

## 性能比较

| 配置 | 典型增益 | 功耗 | 复杂性 |
|-----|---------|------|--------|
| 单天线 | 0 dB（基线） | 最低 | 简单 |
| 双自动 | 3-5 dB | 中等 | 中等 |
| 双固定 | 0-3 dB | 低 | 简单 |

## 高级配置

### 动态切换

```cpp
// 切换到天线 0 进行特定操作
WiFi.setDualAntennaConfig(GPIO_ANT1, GPIO_ANT2, 
                          WIFI_RX_ANT0, WIFI_TX_ANT0);

// 回到自动模式
WiFi.setDualAntennaConfig(GPIO_ANT1, GPIO_ANT2, 
                          WIFI_RX_ANT_AUTO, WIFI_TX_ANT_AUTO);
```

### 监控性能

```cpp
// 比较不同天线配置的扫描结果
int rssi_auto = scanWithConfig(WIFI_RX_ANT_AUTO);
int rssi_ant0 = scanWithConfig(WIFI_RX_ANT0);
int rssi_ant1 = scanWithConfig(WIFI_RX_ANT1);

// 选择最佳配置
```

## 注意事项

- 双天线配置是开发板特定的
- 并非所有 ESP32/芯片变体都支持天线分集
- 配置必须在 WiFi 操作之前设置
- GPIO 引脚特定于你的硬件设计
- 自动模式提供最佳通用性能
- 启用分集后功耗略高
- 示例为加密网络显示 `*`

## 相关示例

- WiFiScan - 无双天线的基本 WiFi 扫描
- WiFiClient - 使用双天线配置连接
- WiFiMulti - 使用双天线的多网络连接

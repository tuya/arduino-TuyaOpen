# WiFi 客户端事件示例

## 概述

本示例演示如何实时监控和处理各种 WiFi 事件。它注册多个事件处理器来跟踪 WiFi 连接状态变化，包括连接、断开、IP 地址分配、身份验证更改等。这对于构建需要响应网络状态变化的强健 WiFi 应用至关重要。

## 功能特性

- **全面事件监控**：跟踪站点模式和接入点模式的所有 WiFi 相关事件
- **多种注册方法**：演示不同的事件处理器注册方式
- **事件信息访问**：展示如何从事件中提取详细信息（如断开原因、IP 地址）
- **动态事件管理**：支持运行时添加和移除事件处理器

## 硬件要求

- 支持 WiFi 功能的 Tuya Open 开发板
- 用于编程和串口监视的 USB 数据线

## 配置说明

上传前，请在程序中修改以下凭据：

```cpp
const char* ssid     = "********";  // 你的 WiFi 网络名称
const char* password = "********";  // 你的 WiFi 密码
```

## 工作原理

### 事件处理器注册

示例演示了三种不同的 WiFi 事件处理器注册方法：

1. **全局事件处理器**：处理所有 WiFi 事件
   ```cpp
   WiFi.onEvent(WiFiEvent);
   ```

2. **特定事件处理器**：只处理特定事件
   ```cpp
   WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
   ```

3. **Lambda 函数处理器**：使用内联 lambda 函数
   ```cpp
   WiFiEventId_t eventID = WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info){
       // 处理事件
   }, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
   ```

### 监控的事件

示例监控各种 WiFi 事件，包括：

**站点模式事件：**
- `ARDUINO_EVENT_WIFI_STA_START`：WiFi 客户端启动
- `ARDUINO_EVENT_WIFI_STA_CONNECTED`：已连接到接入点
- `ARDUINO_EVENT_WIFI_STA_DISCONNECTED`：从接入点断开
- `ARDUINO_EVENT_WIFI_STA_GOT_IP`：获得 IP 地址
- `ARDUINO_EVENT_WIFI_STA_LOST_IP`：失去 IP 地址
- `ARDUINO_EVENT_WIFI_SCAN_DONE`：扫描完成

**接入点模式事件：**
- `ARDUINO_EVENT_WIFI_AP_START`：接入点启动
- `ARDUINO_EVENT_WIFI_AP_STACONNECTED`：客户端已连接
- `ARDUINO_EVENT_WIFI_AP_STADISCONNECTED`：客户端已断开
- `ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED`：为客户端分配 IP

**其他事件：**
- WPS 事件、IPv6 事件、以太网事件

### 事件信息提取

示例展示如何从事件中提取详细信息：

```cpp
void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.println(IPAddress(info.got_ip.ip_info.ip));
}
```

对于断开连接事件，它提取原因代码：
```cpp
Serial.println(info.wifi_sta_disconnected.reason);
```

## 使用方法

1. **配置 WiFi 凭据**：更新 `ssid` 和 `password` 变量
2. **上传程序**：上传到你的 Tuya Open 开发板
3. **打开串口监视器**：设置波特率为 115200
4. **观察事件**：实时查看打印的事件

### 预期输出

```
[WiFi-event] event:
2
WiFi client started
Wait for WiFi...

[WiFi-event] event:
4
Connected to access point

[WiFi-event] event:
7
Obtained IP address: 192.168.1.100
WiFi connected
IP address: 
192.168.1.100
```

## 事件管理

### 添加事件处理器

可以使用 `WiFi.onEvent()` 在任何时候添加事件处理器。该函数返回一个事件 ID，可用于稍后移除处理器。

### 移除事件处理器

要移除事件处理器，请使用事件 ID：

```cpp
WiFiEventId_t eventID = WiFi.onEvent(handler, event);
WiFi.removeEvent(eventID);
```

示例包含一个注释行，展示如何移除事件：
```cpp
// WiFi.removeEvent(eventID);
```

## 应用场景

- **连接监控**：跟踪 WiFi 连接状态以实现自动重连
- **网络诊断**：记录网络事件用于故障排除
- **状态管理**：根据 WiFi 事件更新应用程序状态
- **用户通知**：通知用户连接状态变化
- **自动恢复**：在断开连接时实现重连逻辑
- **接入点管理**：在 AP 模式下监控客户端连接

## 故障排除

### 不显示事件

- 验证 WiFi 凭据是否正确
- 确保开发板在接入点范围内
- 检查串口监视器是否以正确的波特率（115200）打开

### 事件未触发

- 确保在调用 `WiFi.begin()` 之前注册事件处理器
- 验证事件类型是否与你要监控的匹配

### 多次触发事件

- 某些事件在连接过程中可能会触发多次
- 这是正常行为 - 如果需要可以实现防抖动

## 注意事项

- 事件处理器从 WiFi 任务上下文调用，不是主循环
- 保持事件处理器代码轻量级，避免阻塞 WiFi 操作
- 使用事件进行状态跟踪，而不是轮询 `WiFi.status()`
- 示例调用 `WiFi.disconnect(true)` 来清除旧配置

## 相关示例

- WiFiClient - 基本 WiFi 连接
- WiFiMulti - 连接到多个接入点
- WiFiScan - 扫描可用网络

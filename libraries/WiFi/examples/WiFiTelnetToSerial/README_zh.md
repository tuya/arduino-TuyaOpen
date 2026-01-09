# WiFi Telnet 串口桥接

## 概述

本示例创建了 Telnet（网络）和串口（UART）之间的双向桥接。它允许你通过 Telnet 远程连接到设备并与串口控制台交互，本质上创建了一个无线串口终端。这对于远程调试、监控和控制设备非常有用，无需物理连接。

## 功能特性

- **Telnet 服务器**：在端口 23 上接受传入的 Telnet 连接
- **串口桥接**：在 Telnet 客户端和串口之间转发数据
- **多网络支持**：使用 WiFiMulti 自动选择最佳网络
- **双向通信**：数据双向流动（Telnet ↔ 串口）
- **客户端管理**：处理客户端连接和断开
- **连接监控**：自动维护 WiFi 连接

## 硬件要求

- 支持 WiFi 功能的 Tuya Open 开发板
- 用于编程和初始设置的 USB 数据线
- 连接到 UART 的串口设备（可选，用于演示）

## 配置说明

### WiFi 凭据

配置你的 WiFi 网络：

```cpp
const char* ssid     = "********";
const char* password = "********";

wifiMulti.addAP(ssid, password);
wifiMulti.addAP("ssid_from_AP_2", "your_password_for_AP_2");
wifiMulti.addAP("ssid_from_AP_3", "your_password_for_AP_3");
```

### 服务器设置

```cpp
#define MAX_SRV_CLIENTS 1  // 最大同时 Telnet 客户端数
WiFiServer server(23);      // Telnet 端口（默认：23）
```

根据需要调整 `MAX_SRV_CLIENTS` 以允许更多同时连接（增加内存使用）。

## 工作原理

### 系统架构

```
Telnet 客户端 ←→ WiFi ←→ ESP32 ←→ UART ←→ 串口设备
     (PC)                 (桥接)              (可选)
```

### 数据流

**从 Telnet 到串口：**
1. Telnet 客户端发送数据
2. WiFi 接收数据
3. 桥接从 WiFi 客户端读取
4. 桥接写入串口

**从串口到 Telnet：**
1. 串口接收数据
2. 桥接从串口读取
3. 桥接写入所有已连接的 Telnet 客户端
4. WiFi 将数据传输到客户端

### 连接过程

1. **WiFi 连接**：连接到信号最强的可用网络
2. **服务器启动**：在端口 23 上启动 Telnet 服务器
3. **客户端接受**：接受传入的 Telnet 连接
4. **数据转发**：持续双向转发数据
5. **连接维护**：监控和管理客户端连接

### 客户端管理

```cpp
// 接受新客户端
if (server.hasClient()) {
    // 查找空闲槽位
    for(i = 0; i < MAX_SRV_CLIENTS; i++) {
        if (!serverClients[i] || !serverClients[i].connected()) {
            serverClients[i] = server.available();
            break;
        }
    }
}
```

## 使用方法

### 初始设置

1. **配置 WiFi**：更新 WiFi 凭据
2. **上传程序**：上传到你的 Tuya Open 开发板
3. **打开串口监视器**：记下显示的 IP 地址
4. **通过 Telnet 连接**：使用 IP 地址连接

### 使用 Telnet 连接

**Windows (PowerShell/CMD)：**
```
telnet 192.168.1.100 23
```

**Linux/macOS：**
```bash
telnet 192.168.1.100 23
```

**替代方案（netcat）：**
```bash
nc 192.168.1.100 23
```

**PuTTY（Windows GUI）：**
- 主机：192.168.1.100
- 端口：23
- 连接类型：Telnet

### 预期输出

**串口监视器：**
```
Connecting Wifi 
10
9
8
WiFi connected 
IP address: 192.168.1.100
Ready! Use 'telnet 192.168.1.100 23' to connect

New client: 0 192.168.1.50
```

**Telnet 客户端：**
```
Connected to 192.168.1.100
Escape character is '^]'.
[这里输入的所有内容都显示在串口监视器中]
[来自串口的所有内容都显示在这里]
```

## 应用场景

- **远程调试**：无需物理访问即可调试设备
- **无线控制台**：通过网络访问串口控制台
- **远程监控**：远程监控串口输出
- **设备配置**：通过网络而非 USB 配置设备
- **数据记录**：通过网络收集串口数据
- **多用户访问**：多个用户可以监控同一设备（MAX_SRV_CLIENTS > 1）
- **工业自动化**：远程访问设备串口接口
- **物联网开发**：开发期间的无线串口访问

## 高级配置

### 更改 Telnet 端口

```cpp
WiFiServer server(2323);  // 使用端口 2323 而不是 23
```

**注意：** 标准 Telnet 端口是 23。自定义端口需要在客户端中指定端口。

### 多客户端支持

```cpp
#define MAX_SRV_CLIENTS 4  // 允许最多 4 个同时客户端
```

**权衡：** 更多客户端 = 更多内存使用和处理开销。

### 禁用 Nagle 算法

```cpp
server.setNoDelay(true);  // 立即发送数据，减少延迟
```

这减少了延迟，但可能会增加小数据包的网络开销。

### 自定义串口波特率

```cpp
Serial.begin(9600);   // 匹配你的串口设备的波特率
Serial.begin(230400); // 高速串口
```

### 超时配置

为空闲客户端添加超时：

```cpp
if (serverClients[i].connected()) {
    if (millis() - lastActivity[i] > 300000) {  // 5 分钟
        serverClients[i].stop();  // 断开空闲客户端
    }
}
```

## 安全考虑

⚠️ **重要安全警告：**

- Telnet 发送**未加密**的数据（包括密码）
- 网络上的任何人都可以拦截通信
- 默认情况下没有身份验证

### 安全建议

1. **仅在可信网络上使用**：切勿在公共网络上暴露
2. **添加身份验证**：实施登录/密码检查
3. **使用 SSH**：考虑使用 ESP32 SSH 库进行加密
4. **防火墙规则**：限制对特定 IP 地址的访问
5. **VPN**：使用 VPN 进行远程访问
6. **更改默认端口**：使用非标准端口（通过模糊性提供安全性）

### 简单身份验证示例

```cpp
bool authenticated = false;
String password = "secret123";

// 在客户端数据处理中：
if (!authenticated) {
    // 在允许访问前需要密码
    if (receivedData == password) {
        authenticated = true;
        client.println("Authenticated!");
    } else {
        client.stop();
    }
}
```

## 故障排除

### 无法连接到 Telnet 服务器

- 验证设备已连接到 WiFi（检查串口输出）
- 确认 IP 地址正确
- 检查防火墙是否阻止端口 23
- 确保已安装 Telnet 客户端（Windows 上可能需要启用）
- 尝试 ping 以验证网络连接

### 连接立即关闭

- 服务器可能已达到 MAX_SRV_CLIENTS 限制
- 检查串口输出中的客户端槽位可用性
- 如需要，增加 MAX_SRV_CLIENTS

### 数据不显示

- 检查串口波特率是否匹配（本示例中为 115200）
- 验证实际正在发送数据
- 尝试从两个方向发送以隔离问题
- 检查高数据速率下的缓冲区溢出

### 字符乱码

- 设备之间的波特率不匹配
- 串口配置不匹配（位、奇偶校验、停止位）
- 串口线上的电气噪声

### WiFi 持续断开

- 信号强度太弱
- 网络不稳定
- 电源问题
- 检查 WiFi 凭据

### 高延迟

- 网络拥塞
- 尝试 `server.setNoDelay(true)`
- 减少 WiFi 距离
- 检查干扰

## 性能考虑

### 数据速率限制

- 串口：最高 115200 波特率（≈11.5 KB/s）典型
- WiFi：速度快得多，不是瓶颈
- 实际限制取决于串口波特率

### 缓冲区管理

- 串口缓冲区：默认大小不同（通常为 64-128 字节）
- WiFi 缓冲区：由库管理
- 高数据速率可能需要调整缓冲区大小

### 多客户端

每个额外的客户端：
- 增加内存使用
- 增加处理开销
- 可能降低最大数据速率

## 代码流程

```
loop() {
    ├─ 检查 WiFi 连接
    ├─ 接受新 Telnet 客户端
    │   └─ 查找空闲客户端槽位
    ├─ 检查 Telnet 客户端的数据
    │   ├─ 从 Telnet 读取
    │   └─ 写入串口
    ├─ 检查串口的数据
    │   ├─ 从串口读取
    │   └─ 写入所有 Telnet 客户端
    └─ 断开死连接的客户端
}
```

## 注意事项

- 使用 WiFiMulti 实现强健的网络连接
- 端口 23 是标准 Telnet 端口（某些系统上可能需要管理员权限）
- 默认串口波特率为 115200
- 示例使用 `tal_malloc` 和 `tal_free` 进行动态内存管理
- 未实施身份验证 - 生产使用需添加安全性
- 与标准 Telnet 客户端兼容
- 生产环境中考虑使用 SSH 进行加密通信

## 相关示例

- SimpleWiFiServer - 基本 HTTP 服务器
- WiFiClient - 基本 WiFi 客户端连接
- WiFiMulti - 多网络管理
- WiFiClientStaticIP - 为服务器使用静态 IP

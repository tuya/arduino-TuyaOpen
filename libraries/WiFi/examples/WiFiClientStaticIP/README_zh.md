# WiFi 客户端静态 IP 示例

## 概述

本示例演示如何使用静态 IP 地址而非 DHCP 来配置 WiFi 连接。静态 IP 地址在需要一致网络寻址的场景中很有用，例如服务器应用、网络自动化，或使用 MAC 地址过滤和端口转发时。

## 功能特性

- **静态 IP 配置**：配置固定 IP 地址、网关、子网掩码和 DNS 服务器
- **手动网络设置**：完全控制所有网络参数
- **HTTP 客户端示例**：演示使用静态 IP 发起 HTTP 请求
- **网络信息显示**：显示所有已配置的网络参数

## 硬件要求

- 支持 WiFi 功能的 Tuya Open 开发板
- 用于编程和串口监视的 USB 数据线
- 访问路由器设置（确保静态 IP 不与 DHCP 池冲突）

## 配置说明

### WiFi 凭据

更新 WiFi 网络凭据：

```cpp
const char* ssid     = "********";  // 你的 WiFi 网络名称
const char* password = "********";  // 你的 WiFi 密码
```

### 静态 IP 设置

配置你的静态 IP 参数：

```cpp
IPAddress local_IP(192, 168, 15, 69);     // 你想要的静态 IP
IPAddress gateway(192, 168, 15, 1);       // 你的路由器 IP
IPAddress subnet(255, 255, 255, 0);       // 子网掩码
IPAddress primaryDNS(8, 8, 8, 8);         // Google DNS（可选）
```

**重要：** 确保你的静态 IP：
- 在网络子网范围内
- 不与路由器的 DHCP 池冲突
- 网关地址与路由器匹配

### HTTP 请求目标

用于 HTTP 客户端演示：

```cpp
const char* host = "example.com";
const char* url  = "/index.html";
```

## 工作原理

### 静态 IP 配置

程序在连接 WiFi 之前配置静态 IP：

```cpp
if (!WiFi.config(local_IP, gateway, subnet, primaryDNS)) {
    Serial.println("STA Failed to configure");
}
```

必须在 `WiFi.begin()` **之前**调用。

### 连接过程

1. 配置静态 IP 设置
2. 使用凭据开始 WiFi 连接
3. 等待连接建立（WSS_GOT_IP 状态）
4. 显示所有网络信息
5. 定期发起 HTTP 请求以演示连接

### 网络信息显示

成功连接后，程序显示：
- IP 地址（你的静态 IP）
- MAC 地址
- 子网掩码
- 网关 IP
- DNS 服务器

## 使用方法

1. **配置网络设置**：更新静态 IP 配置以匹配你的网络
2. **验证设置**：确保静态 IP 不与 DHCP 冲突
3. **上传程序**：上传到你的 Tuya Open 开发板
4. **打开串口监视器**：设置波特率为 115200
5. **观察连接**：查看网络信息和 HTTP 响应

### 预期输出

```
Connecting to YourNetwork
.....
WiFi connected!
IP address: 192.168.15.69
Mac Address: AA:BB:CC:DD:EE:FF
Subnet Mask: 255.255.255.0
Gateway IP: 192.168.15.1
DNS: 8.8.8.8

connecting to example.com
Requesting URL: /index.html
HTTP/1.1 200 OK
...
closing connection
```

## 网络配置指南

### 选择静态 IP

1. **检查网络范围**：大多数家用路由器使用 192.168.1.x 或 192.168.0.x
2. **避开 DHCP 池**：保留路由器 DHCP 范围之外的范围
   - 例如：如果 DHCP 是 192.168.1.100-200，使用 192.168.1.50
3. **记录你的 IP**：跟踪所有静态 IP 以避免冲突

### 常见网络范围

| 网络类型 | 网关 | 子网掩码 | IP 范围 |
|---------|------|---------|---------|
| C 类（家用） | 192.168.1.1 | 255.255.255.0 | 192.168.1.2-254 |
| C 类替代 | 192.168.0.1 | 255.255.255.0 | 192.168.0.2-254 |
| C 类自定义 | 10.0.0.1 | 255.255.255.0 | 10.0.0.2-254 |

### DNS 配置

示例使用 Google 的公共 DNS（8.8.8.8）。替代方案：
- Google DNS: 8.8.8.8, 8.8.4.4
- Cloudflare: 1.1.1.1, 1.0.0.1
- 你的路由器：通常是网关 IP
- OpenDNS: 208.67.222.222, 208.67.220.220

## 应用场景

- **物联网服务器**：充当服务器的设备需要一致的地址
- **端口转发**：静态 IP 简化端口转发配置
- **MAC 过滤**：某些网络需要预注册的 IP/MAC 对
- **网络监控**：使用静态 IP 更容易跟踪设备
- **工业应用**：自动化系统的固定寻址
- **快速启动**：跳过 DHCP 协商以更快连接

## 故障排除

### 无法连接到 WiFi

- 验证所有网络参数是否正确
- 检查静态 IP 是否在有效范围内
- 确保网关 IP 与路由器匹配
- 确认子网掩码正确（通常为 255.255.255.0）

### "STA Failed to configure" 消息

- 网络参数可能无效
- 尝试再次调用 `WiFi.config()`
- 验证 IPAddress 对象是否正确初始化

### HTTP 请求失败

- 检查 DNS 配置是否正确
- 尝试使用路由器的 IP 作为 DNS 而不是公共 DNS
- 从其他设备验证互联网连接
- 确保防火墙未阻止连接

### IP 冲突

- 另一台设备可能正在使用相同的 IP
- 更改为不同的静态 IP 地址
- 检查路由器的已连接设备列表
- 使用网络扫描工具查找可用的 IP

### 无法到达网关

- 验证网关 IP 是否与路由器匹配
- 检查物理连接和信号强度
- 确保路由器正常工作

## 高级配置

### 使用路由器作为 DNS

```cpp
IPAddress primaryDNS(192, 168, 15, 1);  // 与网关相同
```

### 多个 DNS 服务器

```cpp
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);
WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS);
```

### 动态到静态切换

可以在运行时在 DHCP 和静态之间切换：

```cpp
// 切换到静态
WiFi.config(local_IP, gateway, subnet);

// 切换到 DHCP
WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
```

## 注意事项

- 静态 IP 配置必须在 `WiFi.begin()` 之前完成
- 配置会持续到更改或设备重置
- DNS 参数是可选的但建议设置
- 示例每 5 秒发起一次 HTTP 请求
- 跟踪网络中的所有静态 IP 以避免冲突

## 相关示例

- WiFiClient - 使用 DHCP 的基本 WiFi 连接
- SimpleWiFiServer - 使用静态 IP 创建服务器
- WiFiClientEvents - 监控网络事件

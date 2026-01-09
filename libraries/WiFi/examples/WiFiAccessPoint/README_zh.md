# WiFi 接入点示例

## 概述

本示例演示了如何将设备配置为 WiFi 接入点（AP）。它创建一个简单的 Web 服务器，允许客户端通过 HTTP 请求控制 LED。

## 功能特性

- 创建 WiFi 接入点
- 80 端口的 Web 服务器
- 通过 HTTP GET 请求控制 LED
- 处理多个客户端
- 显示连接状态

## 硬件要求

- 具有 WiFi 功能的涂鸦支持开发板
- 内置 LED（LED_BUILTIN）

## 配置

修改代码中的这些值：

```cpp
const char *ssid = "your_name";       // AP 名称
const char *password = "your_passwd";  // AP 密码（最少 8 个字符）
```

## 工作原理

1. **创建 AP**: 设备创建 WiFi 接入点
2. **启动服务器**: Web 服务器在 80 端口启动
3. **客户端连接**: 设备可以连接到 AP
4. **HTTP 服务器**: 提供带 LED 控制链接的网页
5. **LED 控制**: GET /H 打开 LED，GET /L 关闭 LED

## 使用方法

1. 将固件烧录到设备
2. 打开串口监视器，波特率 115200
3. 设备使用配置的 SSID 创建 AP
4. 将手机/电脑连接到 AP
5. 打开浏览器并导航到显示的 IP（通常是 192.168.4.1）
6. 点击链接控制 LED

## HTTP 端点

- `GET /H` - 打开 LED（HIGH）
- `GET /L` - 关闭 LED（LOW）

## Web 界面

服务器提供简单的 HTML 界面：
```html
点击 <a href="/H">此处</a> 打开 LED。
点击 <a href="/L">此处</a> 关闭 LED。
```

## 示例串口输出

```
Configuring access point...
AP IP address: 192.168.4.1
Server started

New Client.
GET /H HTTP/1.1
...
Client Disconnected.
```

## 网络配置

默认 AP 设置：
- **IP 地址**: 192.168.4.1
- **网关**: 192.168.4.1
- **子网掩码**: 255.255.255.0
- **端口**: 80（HTTP）

## 客户端连接

从任何设备：
1. 搜索 WiFi 网络
2. 连接到你的 AP 名称
3. 输入密码（如果设置）
4. 打开浏览器
5. 导航到 192.168.4.1

## 自定义

### 更改 IP 地址
```cpp
IPAddress local_IP(192,168,4,22);
IPAddress gateway(192,168,4,9);
IPAddress subnet(255,255,255,0);

WiFi.softAPConfig(local_IP, gateway, subnet);
WiFi.softAP(ssid, password);
```

### 开放网络（无密码）
```cpp
WiFi.softAP(ssid);  // 无密码参数
```

### 设置信道和隐藏 SSID
```cpp
WiFi.softAP(ssid, password, channel, hidden);
// channel: 1-13
// hidden: true/false
```

## AP 功能

- **最大客户端数**: 通常为 4-8，取决于硬件
- **范围**: 典型的 WiFi 范围（开放空间 50-100m）
- **安全性**: WPA2-PSK（带密码）
- **信道**: 1-13（取决于地区）

## 安全注意事项

- 始终使用强密码（8+ 个字符）
- 应更改默认密码
- 监控已连接的客户端
- 考虑为 Web 界面实现身份验证

## 常见用例

- IoT 设备配置
- 强制门户
- 本地 Web 界面
- 直接设备控制
- 临时网络共享
- 开发/测试

## 故障排除

### AP 未启动
- 检查 SSID 长度（最多 32 个字符）
- 验证密码长度（WPA 最少 8 个字符）
- 检查是否有冲突的 WiFi 操作

### 无法连接到 AP
- 验证密码是否正确
- 检查是否达到最大客户端数
- 确保设备支持 WiFi 信道
- 尝试重启 AP

### 网页无法加载
- 验证 IP 地址（检查串口输出）
- 确保连接到正确的 AP
- 明确尝试 http://（不是 https://）
- 检查服务器是否成功启动

### LED 无响应
- 验证 LED_BUILTIN 定义正确
- 如果使用外部 LED，检查接线
- 监控串口输出的 GET 请求

## 性能提示

- 保持 AP SSID 广播开启以便于发现
- 使用静态 IP 以保持一致性
- 为不活动的连接实现客户端超时
- 添加错误处理以提高健壮性

## 高级功能

### 站点 + AP 模式
```cpp
WiFi.mode(WIFI_AP_STA);
```

### 获取已连接客户端
```cpp
int clients = WiFi.softAPgetStationNum();
```

### AP 配置
```cpp
wifi_config_t conf;
esp_wifi_get_config(WIFI_IF_AP, &conf);
```

## 依赖库

- WiFi 库
- WiFiClient 库
- WiFiAP 库

## 相关示例

- WiFiClient: 作为客户端连接
- CaptivePortal: 带 DNS 的高级 AP
- SimpleWiFiServer: 基本 Web 服务器

## 注意事项

- AP 和站点模式可以同时运行
- AP IP 默认通常是 192.168.4.1
- Web 服务器非常基础 - 对于生产环境考虑使用 ESPAsyncWebServer
- 记得优雅地处理客户端断开连接

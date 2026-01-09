# 简单 WiFi 服务器示例

## 概述

本示例演示如何创建一个简单的 HTTP 网络服务器，通过网页浏览器控制硬件（LED）。服务器托管一个带有可点击链接的基本网页，用于打开和关闭 LED。这是一个基础示例，展示了如何构建具有基于 Web 的控制界面的物联网设备，可从任何带有网页浏览器的设备访问。

## 功能特性

- **HTTP Web 服务器**：在端口 80 上托管 Web 界面
- **LED 控制**：通过网页点击打开/关闭 LED
- **简单 HTML 界面**：基本但功能齐全的网页
- **GET 请求处理**：响应 HTTP GET 请求
- **网络可访问**：从网络上的任何设备控制
- **实时响应**：即时硬件控制

## 硬件要求

- 支持 WiFi 功能的 Tuya Open 开发板
- 用于编程和串口监视的 USB 数据线
- 连接到 GPIO 5 的 LED（或使用板载 LED，如果可用）

### LED 连接

```
GPIO 5 ----[LED]----[电阻 220Ω]---- GND
```

**注意：** 许多开发板都有内置 LED 可以使用。

## 配置说明

### WiFi 凭据

```cpp
const char* ssid     = "********";  // 你的 WiFi 网络名称
const char* password = "********";  // 你的 WiFi 密码
```

### 硬件引脚

```cpp
pinMode(5, OUTPUT);  // GPIO 5 上的 LED
```

如果你的 LED 连接到不同的引脚，请更改此设置。

### 服务器端口

```cpp
WiFiServer server(80);  // HTTP 端口（默认：80）
```

端口 80 是标准 HTTP。如需要可以使用自定义端口（例如 8080）。

## 工作原理

### HTTP 协议基础

服务器实现了一个最小的 HTTP 服务器：

1. **监听**：在端口 80 上等待传入的 TCP 连接
2. **接受**：接受客户端连接（浏览器）
3. **解析请求**：逐行读取 HTTP 请求
4. **处理命令**：查找 `/H`（LED 开）或 `/L`（LED 关）
5. **发送响应**：返回带有控制链接的 HTML 页面
6. **关闭**：断开客户端连接

### HTML 响应

服务器发送此 HTML 页面：

```html
HTTP/1.1 200 OK
Content-type:text/html

Click <a href="/H">here</a> to turn the LED on pin 5 on.<br>
Click <a href="/L">here</a> to turn the LED on pin 5 off.<br>
```

### 请求处理

```cpp
if (currentLine.endsWith("GET /H")) {
    digitalWrite(5, HIGH);  // 打开 LED
}
if (currentLine.endsWith("GET /L")) {
    digitalWrite(5, LOW);   // 关闭 LED
}
```

## 使用方法

### 设置和连接

1. **连接 LED**：将 LED 连线到 GPIO 5（如果不使用板载）
2. **配置 WiFi**：在程序中更新凭据
3. **上传程序**：上传到你的 Tuya Open 开发板
4. **记录 IP 地址**：打开串口监视器并记录 IP
5. **访问服务器**：打开浏览器并导航到该 IP

### 预期输出

**串口监视器：**
```
Connecting to YourNetwork
...........
WiFi connected.
IP address: 
192.168.1.100

New Client.
GET / HTTP/1.1
Host: 192.168.1.100
Connection: keep-alive
...
Client Disconnected.
```

**网页浏览器：**

导航到 `http://192.168.1.100`

你会看到：
```
Click here to turn the LED on pin 5 on.
Click here to turn the LED on pin 5 off.
```

点击链接控制 LED 并刷新页面。

## Web 界面交互

### 打开 LED

1. 点击 "turn the LED on" 旁边的 "here"
2. 浏览器发送：`GET /H HTTP/1.1`
3. 服务器接收请求
4. 执行：`digitalWrite(5, HIGH)`
5. LED 打开
6. 服务器发送 HTML 响应
7. 浏览器显示相同页面（LED 现在打开）

### 关闭 LED

1. 点击 "turn the LED off" 旁边的 "here"
2. 浏览器发送：`GET /L HTTP/1.1`
3. 服务器接收请求
4. 执行：`digitalWrite(5, LOW)`
5. LED 关闭
6. 服务器发送 HTML 响应

## 应用场景

- **物联网设备控制**：基于 Web 的连接设备控制
- **家庭自动化**：简单的灯光/电器控制
- **远程监控**：从浏览器控制和监控设备
- **原型制作**：快速界面用于测试硬件
- **学习平台**：理解 Web 服务器基础
- **仪表板创建**：更复杂 Web UI 的基础
- **网络诊断**：验证网络连接

## 扩展示例

### 添加状态显示

```cpp
client.print("LED Status: ");
client.print(digitalRead(5) ? "ON" : "OFF");
client.println("<br>");
```

### 多 GPIO 控制

```cpp
if (currentLine.endsWith("GET /LED1ON")) {
    digitalWrite(5, HIGH);
}
if (currentLine.endsWith("GET /LED2ON")) {
    digitalWrite(4, HIGH);
}
```

### 添加 CSS 样式

```cpp
client.println("<!DOCTYPE html><html><head>");
client.println("<style>");
client.println("body { font-family: Arial; margin: 50px; }");
client.println("a { padding: 10px; background: #4CAF50; color: white; text-decoration: none; }");
client.println("</style></head><body>");
client.println("<h1>LED Control</h1>");
client.println("<a href='/H'>Turn ON</a> ");
client.println("<a href='/L'>Turn OFF</a>");
client.println("</body></html>");
```

### JSON API

```cpp
if (currentLine.endsWith("GET /status")) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-type: application/json");
    client.println();
    client.print("{\"led\":\"");
    client.print(digitalRead(5) ? "on" : "off");
    client.println("\"}");
}
```

### 添加传感器读数

```cpp
float temperature = readTemperature();
client.print("Temperature: ");
client.print(temperature);
client.println("°C<br>");
```

## 高级功能

### URL 参数

```cpp
// 解析 URL：/led?state=on
if (currentLine.indexOf("GET /led?state=on") >= 0) {
    digitalWrite(5, HIGH);
}
```

### 表单处理（POST）

```cpp
if (currentLine.startsWith("POST /submit")) {
    // 读取 POST 数据
    // 解析表单字段
}
```

### 多页面

```cpp
if (currentLine.endsWith("GET /")) {
    // 主页
} else if (currentLine.endsWith("GET /settings")) {
    // 设置页面
} else if (currentLine.endsWith("GET /about")) {
    // 关于页面
}
```

### 身份验证

```cpp
bool checkAuth(String request) {
    return request.indexOf("Authorization: Basic") >= 0;
}

if (!checkAuth(currentLine)) {
    client.println("HTTP/1.1 401 Unauthorized");
    client.println("WWW-Authenticate: Basic realm=\"ESP32\"");
    return;
}
```

## 故障排除

### 无法访问网页

- 验证设备已连接到 WiFi（检查串口监视器）
- 确认 IP 地址正确
- 确保设备和计算机在同一网络上
- 检查防火墙设置
- 明确尝试 http://（不是 https://）

### LED 无响应

- 检查 LED 是否正确连接
- 验证 GPIO 引脚编号是否正确
- 在循环中使用简单的 digitalWrite 测试 LED
- 检查 LED 极性（阳极到 GPIO，阴极通过电阻到 GND）

### 页面加载但 LED 不变化

- 验证请求 URL 包含 `/H` 和 `/L`
- 检查串口监视器中接收到的请求
- 确保程序中的逻辑正确
- 使用万用表测试 GPIO

### 响应慢

- 对于简单的阻塞服务器是正常的
- 每个请求必须在下一个之前完成
- 考虑异步 Web 服务器以获得更好的性能

### 连接被拒绝

- 服务器可能未启动（检查 setup()）
- 端口 80 可能正在使用
- 尝试不同的端口（例如 8080）

### 浏览器缓存问题

- 硬刷新（Ctrl+F5 或 Cmd+Shift+R）
- 清除浏览器缓存
- 尝试隐私/无痕窗口

## 安全考虑

⚠️ **安全警告：**

这是一个**演示示例**，没有安全性：

- 无身份验证（任何人都可以控制）
- 无加密（HTTP，不是 HTTPS）
- 无输入验证
- 无 CSRF 保护
- 无速率限制

### 用于生产

1. **添加身份验证**：用户名/密码登录
2. **使用 HTTPS**：加密通信
3. **验证输入**：检查所有参数
4. **实施 CSRF 令牌**：防止跨站攻击
5. **速率限制**：防止滥用
6. **在私有网络上使用**：不要暴露到互联网
7. **防火墙规则**：限制 IP 访问

## 性能说明

- **阻塞服务器**：一次处理一个客户端
- **内存使用**：基本服务器约 10-20KB
- **响应时间**：通常 50-200ms
- **并发客户端**：限于顺序处理
- **升级路径**：考虑 AsyncWebServer 以获得更好的性能

## 代码结构

```
setup() {
    ├─ 初始化串口
    ├─ 配置 GPIO（LED）
    ├─ 连接到 WiFi
    └─ 启动服务器
}

loop() {
    ├─ 检查客户端连接
    ├─ 逐行读取 HTTP 请求
    ├─ 解析 /H 或 /L 命令
    ├─ 根据命令控制 LED
    ├─ 发送 HTTP 响应（HTML）
    └─ 关闭连接
}
```

## 替代实现

### AsyncWebServer（更好的性能）

```cpp
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

server.on("/H", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(5, HIGH);
    request->send(200, "text/html", "LED ON");
});
```

### WebSocket（实时）

对于无需页面刷新的实时更新，考虑 WebSocket 实现。

## 注意事项

- 服务器运行在端口 80（标准 HTTP）
- GPIO 5 上的 LED（根据需要更改）
- 串口波特率：115200
- 使用阻塞套接字操作
- 简单但有效，适合学习和原型制作
- 每次页面加载都会重新连接（无状态 HTTP）
- 浏览器可能会短暂保持连接活动

## 相关示例

- WiFiClient - HTTP 客户端（访问网页）
- WiFiClientStaticIP - 为服务器使用静态 IP
- WiFiTelnetToSerial - Telnet 服务器示例
- WiFiUDPClient - UDP 通信

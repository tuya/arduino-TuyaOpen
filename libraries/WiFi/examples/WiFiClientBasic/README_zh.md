# WiFi 客户端基础示例

## 概述

这是一个基础的 WiFi TCP 客户端示例，演示如何连接到 TCP 服务器、发送请求和接收响应。非常适合学习 Arduino 网络通信的基础知识。

## 功能特性

- WiFi 站点模式连接
- TCP 客户端套接字通信
- 发送 HTTP GET 请求
- 接收和解析服务器响应
- 超时处理
- 连接重试机制

## 硬件要求

- 具有 WiFi 功能的涂鸦 IoT 开发板
- 网络上的 TCP 服务器（或使用 example.com）

## 配置说明

更新 WiFi 凭据和服务器详情：

```cpp
WiFi.begin("your_ssid", "your_password");

const uint16_t port = 80;              // 服务器端口
const char* host = "192.168.1.1";      // 服务器 IP 或主机名
```

## 工作原理

### 连接过程

1. **WiFi 连接**
   - 连接到指定的 SSID
   - 等待 IP 分配
   - 验证连接状态

2. **TCP 连接**
   - 创建 WiFiClient 实例
   - 连接到服务器 IP:端口
   - 实现 5 秒重试延迟

3. **数据交换**
   - 发送 HTTP GET 请求
   - 等待服务器响应（最多 1 秒）
   - 逐行读取响应
   - 关闭连接

## 代码结构

```cpp
void setup() {
    // 初始化串口
    // 连接 WiFi
    // 等待连接
}

void loop() {
    // 创建客户端
    // 连接服务器
    // 发送请求
    // 读取响应
    // 关闭连接
    // 重试前延迟
}
```

## 使用方法

1. 在网络上设置 TCP 服务器
2. 更新 WiFi 凭据
3. 更新主机和端口
4. 上传到开发板
5. 打开串口监视器（115200 波特率）
6. 观察连接和数据交换

## 服务器示例

**简单 HTTP 服务器：**
```cpp
const char* host = "example.com";
const uint16_t port = 80;
client.print("GET /index.html HTTP/1.1\n\n");
```

**自定义 TCP 服务器：**
```cpp
const char* host = "192.168.1.100";
const uint16_t port = 1337;
client.print("Your custom data");
```

## 预期输出

```
Waiting for WiFi... .....
WiFi connected
IP address: 192.168.1.105
Connecting to 192.168.1.1
HTTP/1.1 200 OK
Content-Type: text/html
...
Closing connection.
```

## 故障排除

**WiFi 无法连接：**
- 验证 SSID 和密码
- 检查 WiFi 信号强度
- 确保使用 2.4GHz 网络（如适用）

**服务器连接失败：**
- 验证服务器 IP 和端口
- 检查防火墙设置
- 确保服务器正在运行
- 使用 ping 命令测试

**响应超时：**
- 服务器可能响应慢
- 网络延迟问题
- 增加超时值
- 检查服务器日志

## 高级用法

**发送自定义数据：**
```cpp
client.print("Your custom message");
```

**读取多行：**
```cpp
while(client.available()) {
    String line = client.readStringUntil('\r');
    Serial.println(line);
}
```

**检查连接状态：**
```cpp
if(client.connected()) {
    // 仍然连接
}
```

## 注意事项

- WiFiClient 是非阻塞的
- 连接超时防止挂起
- 始终关闭连接以释放资源
- 适用于 HTTP、SMTP 或自定义协议

## 相关示例

- WiFiClient - ThingSpeak 集成
- SimpleWiFiServer - TCP 服务器
- WiFiClientStaticIP - 静态 IP 配置
- HTTPClient - 完整的 HTTP 客户端库

# WiFi 客户端 - ThingSpeak 示例

## 概述

此示例演示如何使用 WiFi 客户端与 ThingSpeak 物联网平台交互。展示了向通道写入数据和读取数据，提供了用于物联网数据记录和检索的完整 HTTP GET 请求示例。

## 功能特性

- 连接到 WiFi 网络
- 向 ThingSpeak 通道写入传感器数据
- 从 ThingSpeak 通道读取数据
- 解析 JSON 响应
- 处理 HTTP 请求和响应
- 连接超时管理

## 硬件要求

- 具有 WiFi 功能的涂鸦 IoT 开发板
- 有效的互联网连接
- 已配置通道的 ThingSpeak 账户

## 配置说明

更新代码中的以下常量：

```cpp
const char* ssid = "your_wifi_ssid";        // 你的 WiFi 名称
const char* password = "your_wifi_password"; // 你的 WiFi 密码
const String channelID = "your_channel_id";  // 你的 ThingSpeak 通道 ID
const String writeApiKey = "your_write_key"; // 你的写入 API 密钥
const String readApiKey = "your_read_key";   // 你的读取 API 密钥
```

## 工作原理

### 1. WiFi 连接
- 连接到指定的 WiFi 网络
- 等待 IP 地址分配
- 显示连接状态

### 2. 写入数据
- 递增 field1 值
- 发送 HTTP GET 请求更新 ThingSpeak
- 格式：`/update?api_key=KEY&field1=VALUE`

### 3. 读取数据
- 从通道请求最新数据
- 指定结果数量和字段编号
- 检索 JSON 格式的数据

## ThingSpeak 设置

1. 在 [thingspeak.com](https://thingspeak.com) 创建账户
2. 创建新通道
3. 配置 field1（或根据需要添加更多字段）
4. 复制通道 ID 和 API 密钥
5. 在代码中更新这些信息

## 代码结构

```cpp
void setup() {
    // 初始化串口通信
    // 连接 WiFi
    // 显示连接信息
}

void loop() {
    // 向 ThingSpeak 写入数据
    // 从 ThingSpeak 读取数据
    // 等待下一次迭代
}

void readResponse(WiFiClient *client) {
    // 处理服务器响应
    // 实现超时保护
    // 打印接收的数据
}
```

## 使用方法

1. 配置 ThingSpeak 凭据
2. 更新 WiFi 凭据
3. 上传代码到开发板
4. 打开串口监视器（115200 波特率）
5. 监控数据交换
6. 在 ThingSpeak 仪表板查看数据

## API 端点

**写入数据：**
```
GET /update?api_key=WRITEKEY&field1=VALUE HTTP/1.1
Host: api.thingspeak.com
```

**读取数据：**
```
GET /channels/CHANNELID/fields/FIELDNUM.json?results=NUM HTTP/1.1
Host: api.thingspeak.com
```

## 预期输出

```
Connecting to YourWiFi
...
WiFi connected
IP address: 192.168.1.100
HTTP/1.1 200 OK
...
{"channel":{"id":2005329,...
```

## 故障排除

**连接失败：**
- 验证 WiFi 凭据
- 检查互联网连接
- 确保路由器允许 HTTP 流量

**更新失败：**
- 验证写入 API 密钥
- 检查 ThingSpeak 速率限制（更新间隔至少 15 秒）
- 确保字段名称匹配

**读取失败：**
- 验证读取 API 密钥或通道公开设置
- 检查通道 ID
- 验证字段编号存在

## 注意事项

- ThingSpeak 免费账户有更新速率限制
- 通道更新之间最少间隔 15 秒
- 每年最多 300 万条消息
- 如果通道是公开的，数据将公开可见

## 相关示例

- WiFiClientBasic - 简单的 TCP 客户端
- HTTPClient - 完整的 HTTP 客户端库
- WiFiClientStaticIP - 使用静态 IP 配置

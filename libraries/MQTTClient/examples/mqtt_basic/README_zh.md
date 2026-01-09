# MQTT 基础示例

## 功能描述

本示例演示了 MQTTClient 库的基本功能。它连接到 MQTT 服务器，向主题发布消息，订阅另一个主题，并处理接收到的消息。这是一个全面的基础示例，展示了涂鸦开发板上的典型 MQTT 操作。

## 硬件要求

- 任意具有 WiFi 功能的涂鸦支持开发板（T2、T3、T5、ESP32 或 XH_WB5E）
- WiFi 网络连接
- 访问 MQTT 代理服务器（示例使用 broker.emqx.io）

## 使用说明

1. 在 Arduino IDE 中打开本示例
2. 修改 WiFi 凭据：
   ```cpp
   const char *ssid = "your_ssid";
   const char *pass = "your_passwd";
   ```
3. 可选修改 MQTT 设置：
   - `mqtt_broker`: MQTT 代理服务器地址
   - `intopic`: 订阅的主题（接收消息）
   - `outtopic`: 发布的主题（发送消息）
   - `mqtt_username`: MQTT 用户名
   - `mqtt_password`: MQTT 密码
4. 从工具 > 开发板菜单选择您的涂鸦开发板
5. 上传程序到开发板
6. 打开串口监视器，波特率设置为 115200
7. 观察 WiFi 连接和 MQTT 操作过程

## 代码功能要点

- **WiFi 连接**：建立 WiFi 连接并监控状态
- **MQTT 客户端设置**：配置具有基本参数的 MQTT 客户端
- **发布消息**：向输出主题发布测试消息
- **订阅主题**：订阅输入主题以接收消息
- **消息处理**：实现回调函数来处理接收到的消息
- **持续运行**：在主循环中使用 `mqtt.loop()` 维护 MQTT 连接

## 消息回调

`mqtt_message_cb` 函数处理接收到的消息并显示：
- 主题名称
- 有效载荷长度
- 消息内容（作为字符串）

## 配置参数

- **Host（主机）**: broker.emqx.io（公共测试代理）
- **Port（端口）**: 1883（标准 MQTT）
- **Keep-alive（保持连接）**: 15 秒
- **Timeout（超时）**: 100 毫秒
- **Client ID（客户端 ID）**: ty_test_id

## 输出示例

程序运行时，您将看到：
```
...
WiFi connected
IP address: 
192.168.1.100
wifi connected
```

接收到消息时：
```
recv message TopicName: inTopic
payload len: 13
message: Hello World!
```

## 注意事项

- 本示例使用阻塞式 WiFi 连接方法
- loop() 函数中的 `mqtt.loop()` 调用对于维护连接和处理消息至关重要
- 消息假定为字符串，而非二进制数据
- 生产使用时，应实现连接断开时的重连逻辑

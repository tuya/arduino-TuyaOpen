# WiFi 客户端连接 - 连接管理示例

## 概述

此示例演示高级 WiFi 连接管理，包括连接状态监控、超时处理、自动重连控制和断开连接功能。展示了在生产应用中处理 WiFi 连接的最佳实践。

## 功能特性

- 详细的 WiFi 连接状态监控
- 连接尝试超时（10 秒）
- 连接状态机
- 自动重连控制
- 手动断开连接能力
- 全面的错误处理
- 所有连接状态的状态报告

## 硬件要求

- 具有 WiFi 功能的涂鸦 IoT 开发板
- WiFi 路由器/接入点
- 可选：GPIO 8 上的按钮用于手动断开

## 配置说明

更新 WiFi 凭据：

```cpp
const char* ssid = "your_wifi_ssid";
const char* password = "your_wifi_password";
```

## 连接状态

示例监控并报告以下 WiFi 状态：

- `WSS_IDLE` - 未连接
- `WSS_NO_AP_FOUND` - 未找到 SSID
- `WSS_CONN_FAIL` - 连接失败
- `WSS_PASSWD_WRONG` - 密码错误
- `WSS_CONN_SUCCESS` - 连接成功（扫描完成）
- `WSS_GOT_IP` - 获得 IP 地址
- `WSS_DHCP_FAIL` - DHCP 失败

## 工作原理

### 设置阶段

1. **初始化**
   - 配置串口通信
   - 设置 GPIO 按钮输入
   - 开始 WiFi 连接

2. **连接尝试**
   - 以 500ms 间隔尝试连接
   - 最多 20 次尝试（总共 10 秒）
   - 监控状态变化

3. **状态处理**
   - 报告每次状态变化
   - 处理连接失败
   - 成功时显示 IP
   - 超时时强制断开

### 循环阶段

- 监控按钮状态
- 按下按钮时断开 WiFi
- 可扩展重连逻辑

## 代码结构

```cpp
void setup() {
    // 串口初始化
    // 按钮 GPIO 设置
    // 启动 WiFi 连接
    // 监控带超时的连接状态
    // 处理每个状态情况
}

void loop() {
    // 监控按钮
    // 处理断开连接
    // 可添加重连逻辑
}
```

## 自动重连控制

默认情况下，自动重连已启用。要禁用：

```cpp
WiFi.setAutoReconnect(false);
```

**启用（默认）：**
- WiFi 在断开时自动重连
- 无需手动干预
- 适用于大多数应用

**禁用：**
- 需要手动重连
- 对连接生命周期有更多控制
- 适用于功耗敏感的应用

## 使用方法

### 基本使用
1. 更新 WiFi 凭据
2. 上传到开发板
3. 打开串口监视器（115200 波特率）
4. 观察连接过程
5. 查看详细的状态更新

### 使用按钮控制
1. 将按钮连接到 GPIO 8
2. 运行示例
3. 按下按钮断开连接
4. 观察断开行为

## 预期输出

**成功连接：**
```
[WiFi] Connecting to YourSSID
[WiFi] Scan is completed
[WiFi] WiFi is connected!
[WiFi] IP address: 192.168.1.105
```

**未找到 SSID：**
```
[WiFi] Connecting to YourSSID
[WiFi] SSID not found
[WiFi] WiFi Status: 3
[WiFi] Failed to connect to WiFi!
```

**连接超时：**
```
[WiFi] Connecting to YourSSID
[WiFi] WiFi Status: 0
...
[WiFi] Failed to connect to WiFi!
```

## 故障排除

**未找到 SSID：**
- 验证 SSID 拼写
- 检查 WiFi 是否广播 SSID
- 确保使用 2.4GHz 频段（如适用）
- 靠近路由器

**连接失败：**
- 检查密码
- 验证安全类型兼容性
- 检查路由器设置
- 查看 MAC 地址过滤

**未获得 IP：**
- 检查 DHCP 服务器
- 验证网络配置
- 检查路由器 DHCP 池
- 尝试静态 IP 配置

**持续重连：**
- 检查信号强度
- 查找干扰
- 验证路由器稳定性
- 检查电源供应

## 高级功能

### 手动重连

```cpp
void loop() {
    if(WiFi.status() != WSS_GOT_IP) {
        Serial.println("Reconnecting...");
        WiFi.begin(ssid, password);
    }
    delay(10000);
}
```

### 连接超时调整

```cpp
int tryDelay = 1000;      // 每次尝试间隔 1 秒
int numberOfTries = 30;    // 30 次尝试 = 总共 30 秒
```

### 状态回调

```cpp
void WiFiEvent(WiFiEvent_t event) {
    // 处理 WiFi 事件
}
WiFi.onEvent(WiFiEvent);
```

## 最佳实践

1. **始终实现超时** - 防止无限挂起
2. **监控连接状态** - 响应状态变化
3. **处理所有错误情况** - 提供用户反馈
4. **明智使用自动重连** - 考虑功耗
5. **记录状态变化** - 帮助调试

## 注意事项

- 连接尝试使用 500ms 间隔
- 总超时时间 10 秒（20 × 500ms）
- 在循环中检查按钮状态
- 断开是手动的，重连可以是自动的
- 核心调试级别"详细"提供更多详情

## 相关示例

- WiFiClientEvents - 全面的事件处理
- WiFiClientBasic - 简单客户端连接
- WiFiMulti - 多 AP 支持
- WiFiScan - 网络扫描

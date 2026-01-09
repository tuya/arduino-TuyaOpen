# WiFi Multi - 多接入点管理

## 概述

本示例演示如何配置设备从多个接入点列表中自动连接到信号最强的可用 WiFi 网络。WiFiMulti 库智能管理到多个 SSID 的连接，根据信号强度自动选择并连接到最佳可用网络。这对于在多个 WiFi 网络区域运行的移动设备或应用非常理想。

## 功能特性

- **多 AP 支持**：配置到多个 WiFi 网络的连接
- **自动选择**：自动连接到信号最强的可用网络
- **无缝切换**：如果当前连接降级则自动切换网络
- **连接管理**：维护连接并在断开时重新连接
- **基于信号的优先级**：选择具有最佳 RSSI（信号强度）的网络

## 硬件要求

- 支持 WiFi 功能的 Tuya Open 开发板
- 用于编程和串口监视的 USB 数据线
- 访问多个 WiFi 网络（或一个网络用于测试）

## 配置说明

在程序中添加你的 WiFi 网络凭据：

```cpp
wifiMulti.addAP("ssid_from_AP_1", "your_password_for_AP_1");
wifiMulti.addAP("ssid_from_AP_2", "your_password_for_AP_2");
wifiMulti.addAP("ssid_from_AP_3", "your_password_for_AP_3");
```

你可以根据需要添加任意数量的网络。库将按信号强度顺序尝试连接到每个网络。

## 工作原理

### WiFiMulti 库

WiFiMulti 类管理多个 WiFi 凭据并处理：

1. **网络扫描**：定期扫描可用网络
2. **信号评估**：比较可用网络的信号强度（RSSI）
3. **最佳选择**：连接到信号最强的可用网络
4. **自动重连**：维护连接并在断开时重新连接
5. **自动切换**：如果当前网络降级则切换到更好的网络

### 连接过程

```cpp
// 添加网络
wifiMulti.addAP("Network1", "password1");
wifiMulti.addAP("Network2", "password2");

// 尝试连接
if(wifiMulti.run() == WSS_GOT_IP) {
    // 成功连接
}
```

### 连接状态

`wifiMulti.run()` 函数返回 WiFi 状态：
- `WSS_GOT_IP`：成功连接并分配 IP
- 其他值：未连接或连接进行中

### 持续监控

在循环中定期调用 `wifiMulti.run()` 以：
- 维护当前连接
- 检测断开连接
- 如果有更好的网络可用则切换
- 自动重新连接

## 使用方法

1. **添加网络**：配置多个 WiFi 网络凭据
2. **上传程序**：上传到你的 Tuya Open 开发板
3. **打开串口监视器**：设置波特率为 115200
4. **观察连接**：查看连接状态和选定的网络

### 预期输出

**成功连接：**
```
Connecting Wifi...

WiFi connected
IP address: 
192.168.1.100
```

**连接监控：**
```
WiFi not connected!
WiFi not connected!
WiFi connected
IP address: 
192.168.1.100
```

## 连接优先级

### 基于信号的选择

WiFiMulti 根据以下条件选择网络：

1. **可用性**：网络必须在范围内并正在广播
2. **信号强度**：更高的 RSSI（更不负）= 更高的优先级
3. **添加顺序**：如果信号相似，优先选择较早添加的网络

### 示例场景

```
可用网络：
- HomeWiFi:    RSSI -45 dBm（优秀）
- OfficeWiFi:  RSSI -65 dBm（良好）
- GuestWiFi:   RSSI -80 dBm（一般）

选择：HomeWiFi（信号最强）
```

### 网络切换

库可能在以下情况下切换网络：
- 当前连接丢失
- 有明显更强的网络可用
- 当前信号降级到可接受水平以下

## 应用场景

- **移动设备**：在覆盖区域之间移动的机器人或车辆
- **多层建筑**：不同楼层有不同的 AP
- **备份网络**：主网络和后备网络配置
- **公共空间**：商场、机场等的多个 WiFi 选项
- **家庭和办公室**：位置之间的无缝过渡
- **访客网络**：如果主网络不可用则回退到访客网络

## 高级配置

### 动态添加网络

```cpp
// 可以随时添加网络
void addNewNetwork(const char* ssid, const char* password) {
    wifiMulti.addAP(ssid, password);
}
```

### 检查当前连接

```cpp
if(WiFi.status() == WSS_GOT_IP) {
    Serial.println("Connected to: " + WiFi.SSID());
    Serial.println("IP: " + WiFi.localIP().toString());
    Serial.println("RSSI: " + String(WiFi.RSSI()) + " dBm");
}
```

### 快速连接

```cpp
// 设置更短的连接超时
wifiMulti.run(5000);  // 5 秒超时
```

## 故障排除

### 无法连接到任何网络

- 验证至少有一个网络在范围内
- 检查所有网络的凭据是否正确
- 确保 WiFi 已启用且天线已连接
- 检查 SSID 或密码中的拼写错误

### 频繁断开连接

- 网络可能在覆盖范围边缘
- 检查干扰或拥塞
- 验证路由器稳定性
- 考虑添加更近的接入点

### 连接到错误的网络

- 库优先考虑信号强度
- 无法手动设置优先顺序
- 如需特定 AP，考虑使用单网络连接

### 连接慢

- WiFiMulti 扫描所有配置的网络
- 如可能，减少配置的网络数量
- 第一次连接可能比后续连接慢

### 不切换到更好的网络

- 可能需要显著的 RSSI 差异才能触发切换
- 稳定的当前连接优先于稍好的替代方案
- 强制重新连接需要调用 WiFi.disconnect()

## 最佳实践

1. **网络数量**：限制为 3-5 个网络以加快扫描
2. **凭据安全**：安全存储凭据，不要放在代码中
3. **定期监控**：至少每几秒调用一次 `wifiMulti.run()`
4. **连接验证**：始终检查 `run()` 的返回值
5. **日志记录**：记录连接到哪个网络以进行调试

## 代码结构

### 设置阶段

```cpp
void setup() {
    Serial.begin(115200);
    
    // 添加所有网络
    wifiMulti.addAP("Network1", "pass1");
    wifiMulti.addAP("Network2", "pass2");
    
    // 初始连接
    Serial.println("Connecting Wifi...");
    if(wifiMulti.run() == WSS_GOT_IP) {
        Serial.println("WiFi connected");
        Serial.println("IP: " + WiFi.localIP().toString());
    }
}
```

### 循环阶段

```cpp
void loop() {
    // 维护连接
    if(wifiMulti.run() != WSS_GOT_IP) {
        Serial.println("WiFi not connected!");
        delay(1000);
    } else {
        // 连接时执行工作
    }
}
```

## 注意事项

- WiFiMulti 自动处理所有连接管理
- 使用 WiFiMulti 时无需调用 `WiFi.begin()`
- 库持续监控信号强度
- 断开连接时自动重新连接
- 所有配置的网络应使用相同的 WiFi 模式（STA）

## 相关示例

- WiFiClient - 基本单网络连接
- WiFiScan - 手动网络扫描
- WiFiClientEvents - 监控连接事件
- WiFiClientStaticIP - 静态 IP 配置

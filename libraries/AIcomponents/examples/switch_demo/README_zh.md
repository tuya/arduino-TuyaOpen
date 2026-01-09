# 开关演示

## 概述

本示例演示使用涂鸦 IoT 平台实现基本的物联网开关。它展示了如何创建一个可以通过涂鸦智能应用管理的简单可控设备，根据从云端接收的数据点（DP）命令控制 LED。

## 功能特性

- **简单开关控制**：通过涂鸦智能应用控制 LED
- **涂鸦 IoT 集成**：完整的云连接和设备绑定功能
- **数据点处理**：处理多种 DP 类型（布尔、数值、字符串、枚举、位图）
- **LED 指示灯**：通过 LED 控制提供视觉反馈（GPIO 1）
- **设备许可证管理**：支持板载许可证和手动配置
- **串口通信**：用于调试的实时串口监视器

## 硬件要求

- 涂鸦兼容的开发板（ESP32、T2、T3、T5、LN882H 或 XH_WB5E 系列）
- 连接到 GPIO 1 的 LED（用于开关状态指示）
- 稳定的 Wi-Fi 连接
- 串口控制台用于监控（波特率 115200）

## 配置说明

上传代码前，需要配置以下参数：

```cpp
#define TUYA_DEVICE_UUID "uuidxxxxxxxxxxxxxxxx"
#define TUYA_DEVICE_AUTHKEY "keyxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
#define TUYA_PRODUCT_ID "alon7qgyjj8yus74"
```

您可以在 [涂鸦开发者平台](https://tuyaopen.ai/zh/docs/cloud/tuya-cloud/creating-new-product) 创建产品获得 PID 或者使用默认的 PID。

关于授权码的获取方式，参考 [授权码获取](https://tuyaopen.ai/zh/docs/quick-start#tuyaopen-%E6%8E%88%E6%9D%83%E7%A0%81%E8%8E%B7%E5%8F%96).
您必须替换自己的设备 UUID 和 AUTHKEY。

示例会自动尝试从板载许可证读取凭证。如果不可用，将使用上面定义的值。

## 使用方法

1. **上传代码**：将代码烧录到开发板
2. **打开串口监视器**：设置波特率为 115200 以查看调试信息
3. **设备绑定**：
   - 给设备上电
   - 打开[涂鸦智能应用](https://tuyaopen.ai/zh/docs/quick-start/device-network-configuration)
   - 添加新设备并按照配对说明操作
4. **控制开关**：
   - 使用涂鸦智能应用切换开关状态
   - GPIO 1 上的 LED 将相应地开启/关闭
   - 在串口监视器中监控事件

### LED 行为

- **LED 熄灭（高电平）**：开关关闭
- **LED 点亮（低电平）**：开关打开

## 核心功能

### 主要组件

- **`setup()`**：初始化串口通信、GPIO 引脚和涂鸦 IoT 客户端
- **`loop()`**：维护串口通信并检查传入数据
- **`tuyaIoTEventCallback()`**：处理所有 IoT 平台事件

### 事件处理

回调函数处理各种涂鸦 IoT 事件：

#### 设备生命周期事件
- **`TUYA_EVENT_BIND_START`**：设备配对开始
- **`TUYA_EVENT_DIRECT_MQTT_CONNECTED`**：建立直连 MQTT 连接（如果启用则显示二维码）
- **`TUYA_EVENT_MQTT_CONNECTED`**：云连接成功，设备在线
- **`TUYA_EVENT_RESET`**：请求设备重置

#### 数据点事件
- **`TUYA_EVENT_DP_RECEIVE_OBJ`**：接收对象类型数据点
  - **PROP_BOOL**：布尔值（开关 ON/OFF）
  - **PROP_VALUE**：整数值
  - **PROP_STR**：字符串值
  - **PROP_ENUM**：枚举值
  - **PROP_BITMAP**：位图标志
- **`TUYA_EVENT_DP_RECEIVE_RAW`**：接收原始数据点

#### 其他事件
- **`TUYA_EVENT_UPGRADE_NOTIFY`**：OTA 升级通知
- **`TUYA_EVENT_TIMESTAMP_SYNC`**：与云端时间同步

### 数据点处理

当接收到布尔类型 DP 时：
```cpp
if (dp->value.dp_bool == true) {
    digitalWrite(APP_LED_PIN, LOW);  // LED 点亮
} else {
    digitalWrite(APP_LED_PIN, HIGH); // LED 熄灭
}
```

处理后，DP 值会上报回云端以同步状态。

## 项目结构

```
switch_demo/
├── switch_demo.ino    # 主程序文件
└── README.md          # 本文档
```

## 故障排除

- **设备无法连接**：
  - 验证网络配置中的 Wi-Fi 凭证
  - 检查涂鸦账户是否正确设置
  - 确保设备 UUID 和 AUTHKEY 正确

- **LED 无响应**：
  - 检查 GPIO 1 连接
  - 验证 LED 正确连接
  - 监控串口输出以确认 DP 接收

## 注意事项

- 如果重置按钮被按下三次，设备将自动重置网络配置
- GPIO 1 上的 LED 指示设备状态
- 本示例需要有效的涂鸦 IoT 账户和正确配置的产品

## 相关文档

 - [TuyaOpen 官网](https://tuyaopen.ai)
 - [Github 代码库](https://github.com/tuya/TuyaOpen)
 - [Arduino-TuyaOpen 仓库](https://github.com/tuya/Arduino-TuyaOpen)
 - [设备配网](https://tuyaopen.ai/zh/docs/quick-start/device-network-configuration) (需下载 APP)

## 联系我们

您有任何好的建议和反馈，欢迎在 [Github](https://github.com/tuya/TuyaOpen/issues) 提交 issue ！！！

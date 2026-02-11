# IoT 基础示例 - 开关控制

本示例演示如何使用 Arduino-TuyaOpen 框架快速实现一个基于涂鸦 IoT 平台的智能开关设备。用户可通过涂鸦智能 APP 远程控制开发板上的 LED 开关，并了解涂鸦 IoT 平台的 DP（Data Point）数据交互机制。

> 本示例不涉及 AI 功能，适用于所有涂鸦支持的开发板平台（ESP32、T2、T3、TUYA_T5AI、LN882H、XH_WB5E 等）。

## 代码烧录流程

0. 确保已完成[快速开始](Quick_start.md)中开发环境的搭建。

1. 连接开发板到电脑，打开 Arduino IDE，选择对应的开发板型号，并选择正确的烧录端口。

2. 在 Arduino IDE 中点击 `文件` -> `示例` -> `AI components` -> `00_IoT_SimpleExample`，打开示例代码。

3. 将示例文件中的授权码信息替换为自己的信息。
    - [什么是授权码](https://tuyaopen.ai/zh/docs/quick-start#tuyaopen-%E4%B8%93%E7%94%A8%E6%8E%88%E6%9D%83%E7%A0%81)
    - [如何获取授权码](https://tuyaopen.ai/zh/docs/quick-start#tuyaopen-%E6%8E%88%E6%9D%83%E7%A0%81%E8%8E%B7%E5%8F%96)

```cpp
// 设备授权码（替换为自己的授权码）
#define TUYA_DEVICE_UUID    "uuidxxxxxxxxxxxxxxxx"
#define TUYA_DEVICE_AUTHKEY "keyxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
// 产品 PID（可使用默认值，或替换为涂鸦 IoT 平台上创建的产品 PID）
#define TUYA_PRODUCT_ID     "qhivvyqawogv04e4"
```

4. 点击 Arduino IDE 左上角按钮烧录代码，终端出现以下信息说明烧录成功。

```bash
[INFO]: Write flash success
[INFO]: CRC check success
[INFO]: Reboot done
[INFO]: Flash write success.
```

## 设备连接与交互

### 设备连接

烧录固件后的开发板需要连接网络并注册到云端，使设备具备远程通信能力。配网后即可实现 `手机` - `涂鸦云` - `设备` 三方通信。

设备配网流程：[设备配网](https://tuyaopen.ai/zh/docs/quick-start/device-network-configuration)

### 设备交互

配网成功后，可通过涂鸦智能 APP 控制开关 DP，开发板上的 LED 将根据指令亮灭。同时可打开 Arduino IDE 串口监视器（波特率 115200）查看设备运行日志。

#### LED 行为

| 状态 | LED |
| :---: | --- |
| APP 下发开关 `true` | LED 点亮（GPIO 低电平） |
| APP 下发开关 `false` | LED 熄灭（GPIO 高电平） |

## 示例代码说明

### 硬件配置

```cpp
#define APP_LED_PIN  1   // LED 引脚号（GPIO 1）
```

### IoT 初始化

```cpp
// 设置 IoT 事件回调函数
TuyaIoT.setEventCallback(tuyaIoTEventCallback);

// 设置设备授权码（优先尝试从板载读取，失败则使用宏定义值）
tuya_iot_license_t license;
int rt = TuyaIoT.readBoardLicense(&license);
if (OPRT_OK != rt) {
    license.uuid = (char *)TUYA_DEVICE_UUID;
    license.authkey = (char *)TUYA_DEVICE_AUTHKEY;
}
TuyaIoT.setLicense(license.uuid, license.authkey);

// 启动 IoT 服务
TuyaIoT.begin(TUYA_PRODUCT_ID, PROJECT_VERSION);
```

> 本示例通过 `TuyaIoT.readBoardLicense()` 尝试从板载存储读取授权码。若读取失败，则回退到代码中宏定义的默认值。

### IoT 事件处理

所有 IoT 平台事件在 `tuyaIoTEventCallback` 中统一处理，主要事件包括：

| 事件 | 说明 |
| --- | --- |
| `TUYA_EVENT_BIND_START` | 设备开始配网 |
| `TUYA_EVENT_MQTT_CONNECTED` | MQTT 连接成功，设备上线 |
| `TUYA_EVENT_TIMESTAMP_SYNC` | 云端时间同步 |
| `TUYA_EVENT_RESET` | 设备被重置 |
| `TUYA_EVENT_UPGRADE_NOTIFY` | 收到 OTA 升级通知 |
| `TUYA_EVENT_DP_RECEIVE_OBJ` | 收到对象型 DP 数据 |
| `TUYA_EVENT_DP_RECEIVE_RAW` | 收到原始型 DP 数据 |

### DP 数据交互

涂鸦 IoT 通过 **DP（Data Point）** 实现云端与设备间的数据交互。本示例演示了对象型 DP 的完整处理流程：

#### 接收 DP 数据

当 APP 下发指令时，设备端通过 `TUYA_EVENT_DP_RECEIVE_OBJ` 事件接收 DP 数据，并根据 DP 类型分别处理：

| DP 类型 | 宏 | 说明 |
| --- | --- | --- |
| 布尔型 | `PROP_BOOL` | 开关量，如 LED 开/关 |
| 整数型 | `PROP_VALUE` | 数值量，如亮度、温度 |
| 字符串型 | `PROP_STR` | 文本数据 |
| 枚举型 | `PROP_ENUM` | 有限选项，如模式选择 |
| 位图型 | `PROP_BITMAP` | 位标志数据 |

核心处理逻辑：

```cpp
case PROP_BOOL:
    if (dp->value.dp_bool == true) {
        digitalWrite(APP_LED_PIN, LOW);   // LED 点亮
    } else {
        digitalWrite(APP_LED_PIN, HIGH);  // LED 熄灭
    }
    break;
```

#### 上报 DP 数据

处理完 DP 后需调用 `TuyaIoT.write()` 将状态上报云端，保持 APP 与设备状态同步：

```cpp
// 上报对象型 DP
TuyaIoT.write((dpobj->dps->id), (dpobj->dps->value), 0);

// 上报原始型 DP
TuyaIoT.write((dpraw->dp.id), (dpraw->dp.data), (dpraw->dp.len), 3);
```

## 相关文档

- [TuyaOpen 官网](https://tuyaopen.ai)
- [涂鸦云产品创建](https://tuyaopen.ai/zh/docs/cloud/tuya-cloud/creating-new-product)
- [Github 代码库](https://github.com/tuya/TuyaOpen)

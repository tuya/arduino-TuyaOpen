# Home

![Spell Check](https://img.shields.io/github/actions/workflow/status/tuya/arduino-tuyaopen/spell-check.yml?style=plastic&label=Spell%20Check)

[English](README.md) | [简体中文](README_zh.md)

## 简介

arduino-tuyaopen 是 tuya 针对 Arduino 平台提供的开源 IoT 开发框架，让众多喜欢 Arduino 的开发者可以快速开发出基于涂鸦云平台的物联网设备，实现设备远程智能控制。

arduino-tuyaopen 基于 [TuyaOpen](https://github.com/tuya/TuyaOpen) 开发，TuyaOpen 是一款跨芯片平台、操作系统的 AIoT 开发框架。它基于通用南向接口设计，支持 Bluetooth、Wi-Fi、Ethernet 等通信协议，提供了物联网开发的核心功能，包括配网，激活，控制，升级等；它具备强大的安全合规能力，包括设备认证、数据加密、通信加密等，满足全球各个国家和地区的数据合规需求。

基于 TuyaOpen 开发的 IoT 产品，如果使用 tuya_cloud_service 组件的功能，就可以使用涂鸦APP、云服务提供的强大生态能力，并与 Power By Tuya 设备互联互通。

同时 TuyaOpen 将不断拓展，提供更多云平台接入功能，及语音、视频、人脸识别等功能，arduino-tuyaopen 也会同步更新支持更多功能。

## 支持的芯片
目前该项目已支持 T2，T3 和 T5 芯片。
| 芯片 |                          Datasheet                           |
| :--: | :----------------------------------------------------------: |
|  T2  | [T2](https://developer.tuya.com/cn/docs/iot/T2-U-module-datasheet?id=Kce1tncb80ldq) |
|  T3  | [T3](https://developer.tuya.com/cn/docs/iot/T3-U-Module-Datasheet?id=Kdd4pzscwf0il) |
|  TUYA_T5AI  | [TUYA_T5AI](https://developer.tuya.com/cn/docs/iot/T5-E1-Module-Datasheet?id=Kdar6hf0kzmfi) |
|  LN882H  | LN882H|
|  ESP32  | [ESP32](https://www.espressif.com/en/products/socs/esp32/resources) |

## 支持的操作系统

| 芯片 | Windows  | Linux |  macOS   |
| :--: | :------: | :---: | :------: |
|  T2  |   支持   | 支持  |  暂不支持  |
|  T3  |   支持   | 支持  |   支持    |
|  TUYA_T5AI  |   支持   | 支持  |   支持    |
|  LN882H  |   支持   | 支持  |   支持    |
|  ESP32  |   支持   | 支持  |   支持    |

> 注意：当前部分芯片未支持某些操作系统，我们正在全力支持中，敬请期待！


## arduino-tuyaopen 使用 

推荐安装使用最新版本的 Arduino IDE 2，可点击 arduino 官网 [arduino.cc](https://www.arduino.cc/en/software) 下载 Arduino IDE，该项目的所有编译烧录测试都是在 Arduino IDE 2 上进行的。

+ 复制下方开发板管理地址：

  ```
  https://github.com/tuya/arduino-tuyaopen/releases/download/global/package_tuya_open_index.json
  ```

  或使用 gitee 地址：

  ```
  https://gitee.com/tuya-open/arduino-tuyaopen/releases/download/global/package_tuya_open_index_cn.json
  ```

+ 启动 Arduino IDE 2，并依次点击 “文件” -> “首选项” ，打开首选项窗口。

  ![Preferences](https://images.tuyacn.com/fe-static/docs/img/9f354b85-5f9d-4af6-be60-c114d7b1e822.png)

+ 在 “其他开发板管理器地址” 中输入上述开发板管理地址。

  ![BoardManagerURL](https://images.tuyacn.com/fe-static/docs/img/3e53a3ce-c603-481a-9ac0-fd20d6fa6525.png)

+ 在 “开发板管理器”，输入 "Tuya Open" 查找并安装最新版本。

## 如何使用 arduino-tuyaopen 进行云连接

+ [连接 Tuya IoT 平台](./libraries/TuyaIoT/examples/quickStart/README_zh.md)

## 开发板硬件简介

+ [T2-U 开发板](https://developer.tuya.com/cn/docs/iot/t2-u-board?id=Kce6cq9e9vlmv)
+ [T3-U 模组规格书](https://developer.tuya.com/cn/docs/iot/T3-U-Module-Datasheet?id=Kdd4pzscwf0il)
+ [T5-E1 模组规格书](https://developer.tuya.com/cn/docs/iot/T5-E1-Module-Datasheet?id=Kdar6hf0kzmfi)

## 支持

在使用该项目的过程中，如果您遇到问题或者有新的功能需求或想法，可以在该项目下通过 issues 的方式进行沟通交流。

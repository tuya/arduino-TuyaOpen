# BLE 服务器示例

## 概述
此示例演示如何使用涂鸦 BLE 库创建低功耗蓝牙（BLE）GATT 服务器。它展示了如何设置 BLE 服务、特征、广播，以及如何使用通知和指示处理客户端连接。

## 功能特性
- **BLE GATT 服务器**：创建具有自定义服务和特征的完整 BLE 服务器
- **多个特征**：演示 READ、WRITE、NOTIFY 和 INDICATE 属性
- **自定义广播**：可配置的广播和扫描响应数据
- **连接管理**：处理连接/断开连接事件
- **自动递增计数器**：发送递增值以演示通知和指示
- **回调系统**：用于处理 BLE 事件的事件驱动架构

## 硬件要求
- 支持 BLE 的涂鸦开发板（T2、T3、ESP32 或具有 BLE 功能的兼容板）
- 支持 BLE 的测试设备（智能手机、平板电脑或带 BLE 的计算机）
- 推荐的 BLE 测试应用：
  - **Android**：nRF Connect、BLE Scanner
  - **iOS**：nRF Connect、LightBlue
  - **桌面**：nRF Connect for Desktop

## BLE 配置

### 服务和特征
- **服务 UUID**：`4fafc201-1fb5-459e-8fcc-c5c9c331914b`
- **特征 1 UUID**：`FF11`
  - 属性：READ、WRITE、NOTIFY
  - 用于双向通信和通知
- **特征 2 UUID**：`FF22`
  - 属性：WRITE、INDICATE
  - 用于写操作和指示

### 广播数据
- **设备名称**：`TEST_BLE`
- **扫描响应**：`TEST`

## 使用说明

### 1. 上传并运行
1. 将程序上传到您的涂鸦开发板
2. 打开串口监视器，波特率设置为 115200
3. 开发板将自动开始以 "TEST_BLE" 的名称进行广播

### 2. 使用 BLE 客户端连接
1. 在手机或电脑上打开 BLE 扫描应用
2. 扫描 BLE 设备并查找 "TEST_BLE"
3. 连接到该设备

### 3. 与特征交互

**使用 nRF Connect：**
1. 连接后，您将看到包含两个特征的服务
2. 在特征 1（UUID：FF11）上**启用通知**
3. 在特征 2（UUID：FF22）上**启用指示**
4. 您将每秒收到自动递增的计数器值
5. **写入数据**到任一特征以测试写回调

### 4. 监控串口输出
串口监视器将显示：
- 连接状态：客户端连接时显示 "onConnect..."
- 断开连接状态：客户端断开连接时显示 "onDisconnect..."
- 写入事件：写入数据时显示数据长度和第一个字节值
- 通知完成：每次通知后显示 "notify_done..."

## 代码功能要点

### 1. 服务器初始化
```cpp
BLEDEV::init();
BLEServer* pServer = BLEDEV::createServer();
pServer->setCallbacks(new MyServerCallbacks());
```
初始化 BLE 设备并创建具有回调处理的服务器。

### 2. 服务和特征创建
```cpp
BLEService *pService = pServer->createService(SERVICE_UUID);
pCharacteristic1 = pService->createCharacteristic(
    CHARACTERISTIC_UUID1,
    BLECharacteristic::PROPERTY_READ | 
    BLECharacteristic::PROPERTY_WRITE | 
    BLECharacteristic::PROPERTY_NOTIFY
);
```
创建支持多个属性的服务和特征。

### 3. 自定义广播
```cpp
adv_data[0] = 0x0A;           // 长度
adv_data[1] = 0x09;           // 完整本地名称
memcpy(&adv_data[2], "TEST_BLE", 9);
pAdvertising->setAdvertisementData((uint8_t*)&adv_data, 0xb);
```
使用设备名称配置自定义广播数据。

### 4. 通知和指示
```cpp
pCharacteristic1->setValue((uint8_t*)&value, 4);
pCharacteristic1->notify();  // 发送通知

pCharacteristic2->setValue((uint8_t*)&value, 4);
pCharacteristic2->indicate();  // 发送指示
```
演示使用通知（无确认）和指示（有确认）向连接的客户端发送数据。

### 5. 连接状态管理
示例跟踪连接状态并在断开连接时自动重新开始广播：
```cpp
if (!deviceConnected && oldDeviceConnected) {
    delay(500);
    pAdvertising->start();  // 重新开始广播
}
```

## BLE 概念

### 通知 vs 指示
- **通知**：快速、单向数据推送，无需确认
- **指示**：可靠的数据推送，需要客户端确认
- 使用通知进行高频数据传输（传感器、流式传输）
- 使用指示传输必须接收的关键数据

### 特征属性
- **READ**：客户端可以读取值
- **WRITE**：客户端可以写入值
- **NOTIFY**：服务器可以推送数据而无需确认
- **INDICATE**：服务器可以推送数据并需要确认

## 时序考虑
示例在通知和指示之间使用 500 毫秒延迟以防止 BLE 堆栈拥塞。对于生产环境：
- 根据数据速率要求调整时序
- 监控 BLE 连接间隔
- 考虑连接参数优化
- 最小安全间隔：约 3 毫秒（经过 6 小时测试）

## 故障排除

**找不到设备：**
- 确保测试设备上已启用 BLE
- 检查板是否支持 BLE（查看板规格确认）
- 尝试重置板并重新扫描

**无法连接：**
- 确保一次只有一个设备尝试连接
- 如果板处于错误状态，请重置板
- 检查串口监视器的错误消息

**未收到通知：**
- 验证您是否在 BLE 客户端应用中启用了通知/指示
- 检查连接是否仍处于活动状态
- 确保特征支持所需的属性

**写操作失败：**
- 验证特征是否启用了 WRITE 属性
- 检查数据格式和长度
- 监控串口输出的错误消息

## 示例串口输出
```
onConnect...
notify_done...
notify_done...
 data_len: 4 data: 100
notify_done...
onDisconnect...
start advertising
onConnect...
```

## 扩展此示例

### 添加更多特征
为不同的数据类型创建额外的特征：
- 温度读数
- 按钮状态
- 配置参数

### 实现正确的数据解析
用正确的数据解析替换简单的 `data[0]` 读取：
```cpp
void onwrite(uint8_t* data, uint16_t data_len) {
    if (data_len >= 4) {
        uint32_t value = *(uint32_t*)data;
        // 处理值
    }
}
```

# BLE Server Example

## Overview
This example demonstrates how to create a Bluetooth Low Energy (BLE) GATT server using the Tuya BLE library. It shows how to set up BLE services, characteristics, advertising, and handle client connections with notifications and indications.

## Features
- **BLE GATT Server**: Creates a complete BLE server with custom service and characteristics
- **Multiple Characteristics**: Demonstrates READ, WRITE, NOTIFY, and INDICATE properties
- **Custom Advertising**: Configurable advertisement and scan response data
- **Connection Management**: Handles connect/disconnect events
- **Auto-increment Counter**: Sends incremental values to demonstrate notifications and indications
- **Callback System**: Event-driven architecture for handling BLE events

## Hardware Requirements
- Tuya development board with BLE support (T2, T3, ESP32, or compatible boards with BLE capability)
- BLE-enabled device for testing (smartphone, tablet, or computer with BLE)
- Recommended BLE testing apps:
  - **Android**: nRF Connect, BLE Scanner
  - **iOS**: nRF Connect, LightBlue
  - **Desktop**: nRF Connect for Desktop

## BLE Configuration

### Service and Characteristics
- **Service UUID**: `4fafc201-1fb5-459e-8fcc-c5c9c331914b`
- **Characteristic 1 UUID**: `FF11`
  - Properties: READ, WRITE, NOTIFY
  - Used for bidirectional communication and notifications
- **Characteristic 2 UUID**: `FF22`
  - Properties: WRITE, INDICATE
  - Used for write operations and indications

### Advertising Data
- **Device Name**: `TEST_BLE`
- **Scan Response**: `TEST`

## Usage Instructions

### 1. Upload and Run
1. Upload the sketch to your Tuya development board
2. Open Serial Monitor at 115200 baud
3. The board will automatically start advertising as "TEST_BLE"

### 2. Connect with BLE Client
1. Open a BLE scanning app on your phone or computer
2. Scan for BLE devices and look for "TEST_BLE"
3. Connect to the device

### 3. Interact with Characteristics

**Using nRF Connect:**
1. After connecting, you'll see the service with two characteristics
2. **Enable notifications** on Characteristic 1 (UUID: FF11)
3. **Enable indications** on Characteristic 2 (UUID: FF22)
4. You'll receive auto-incrementing counter values every second
5. **Write data** to either characteristic to test the write callback

### 4. Monitor Serial Output
The Serial Monitor will display:
- Connection status: "onConnect..." when client connects
- Disconnection status: "onDisconnect..." when client disconnects
- Write events: Data length and first byte value when data is written
- Notification completion: "notify_done..." after each notification

## Key Code Features

### 1. Server Initialization
```cpp
BLEDEV::init();
BLEServer* pServer = BLEDEV::createServer();
pServer->setCallbacks(new MyServerCallbacks());
```
Initializes BLE device and creates server with callback handling.

### 2. Service and Characteristic Creation
```cpp
BLEService *pService = pServer->createService(SERVICE_UUID);
pCharacteristic1 = pService->createCharacteristic(
    CHARACTERISTIC_UUID1,
    BLECharacteristic::PROPERTY_READ | 
    BLECharacteristic::PROPERTY_WRITE | 
    BLECharacteristic::PROPERTY_NOTIFY
);
```
Creates a service with characteristics supporting multiple properties.

### 3. Custom Advertising
```cpp
adv_data[0] = 0x0A;           // Length
adv_data[1] = 0x09;           // Complete local name
memcpy(&adv_data[2], "TEST_BLE", 9);
pAdvertising->setAdvertisementData((uint8_t*)&adv_data, 0xb);
```
Configures custom advertising data with device name.

### 4. Notifications and Indications
```cpp
pCharacteristic1->setValue((uint8_t*)&value, 4);
pCharacteristic1->notify();  // Send notification

pCharacteristic2->setValue((uint8_t*)&value, 4);
pCharacteristic2->indicate();  // Send indication
```
Demonstrates sending data to connected clients using notifications (unacknowledged) and indications (acknowledged).

### 5. Connection State Management
The example tracks connection state and automatically restarts advertising when disconnected:
```cpp
if (!deviceConnected && oldDeviceConnected) {
    delay(500);
    pAdvertising->start();  // Restart advertising
}
```

## BLE Concepts

### Notifications vs Indications
- **Notifications**: Fast, one-way data push without acknowledgment
- **Indications**: Reliable data push with acknowledgment from client
- Use notifications for high-frequency data (sensors, streaming)
- Use indications for critical data that must be received

### Characteristic Properties
- **READ**: Client can read the value
- **WRITE**: Client can write a value
- **NOTIFY**: Server can push data without acknowledgment
- **INDICATE**: Server can push data with acknowledgment

## Timing Considerations
The example uses 500ms delays between notifications and indications to prevent BLE stack congestion. For production:
- Adjust timing based on data rate requirements
- Monitor BLE connection interval
- Consider connection parameter optimization
- Minimum safe interval: ~3ms (tested for 6 hours)

## Troubleshooting

**Cannot find device:**
- Ensure BLE is enabled on your testing device
- Check if board supports BLE (verify with board specifications)
- Try resetting the board and scanning again

**Cannot connect:**
- Ensure only one device is trying to connect at a time
- Reset the board if it's in an error state
- Check Serial Monitor for error messages

**No notifications received:**
- Verify you've enabled notifications/indications in your BLE client app
- Check that the connection is still active
- Ensure the characteristic supports the desired property

**Write operation fails:**
- Verify the characteristic has WRITE property enabled
- Check data format and length
- Monitor Serial output for error messages

## Example Serial Output
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

## Extending This Example

### Add More Characteristics
Create additional characteristics for different data types:
- Temperature readings
- Button states
- Configuration parameters

### Implement Proper Data Parsing
Replace the simple `data[0]` read with proper data parsing:
```cpp
void onwrite(uint8_t* data, uint16_t data_len) {
    if (data_len >= 4) {
        uint32_t value = *(uint32_t*)data;
        // Process value
    }
}
```
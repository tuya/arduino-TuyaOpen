# IoT Basic Example - Switch Control

This example demonstrates how to quickly implement a smart switch device based on the Tuya IoT platform using the Arduino-TuyaOpen framework. Users can remotely control the LED on the development board via the Tuya Smart APP, and learn about the DP (Data Point) data interaction mechanism of the Tuya IoT platform.

> This example does not involve AI features and is compatible with all Tuya-supported development board platforms (ESP32, T2, T3, TUYA_T5AI, LN882H, XH_WB5E, etc.).

## Flashing Procedure

0. Make sure you have completed the development environment setup described in [Quick Start](Quick_start.md).

1. Connect the development board to your computer, open Arduino IDE, select the corresponding board model, and choose the correct upload port.

2. In Arduino IDE, click `File` -> `Examples` -> `AI components` -> `00_IoT_SimpleExample` to open the example code.

3. Replace the authorization code in the example file with your own credentials.
    - [What is an authorization code](https://tuyaopen.ai/en/docs/quick-start#tuyaopen-authorization-code)
    - [How to obtain an authorization code](https://tuyaopen.ai/en/docs/quick-start#obtaining-tuyaopen-authorization-code)

```cpp
// Device authorization code (replace with your own)
#define TUYA_DEVICE_UUID    "uuidxxxxxxxxxxxxxxxx"
#define TUYA_DEVICE_AUTHKEY "keyxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
// Product PID (use the default value, or replace with the PID created on the Tuya IoT platform)
#define TUYA_PRODUCT_ID     "qhivvyqawogv04e4"
```

4. Click the upload button in the top-left corner of Arduino IDE to flash the code. The following messages in the terminal indicate a successful flash.

```bash
[INFO]: Write flash success
[INFO]: CRC check success
[INFO]: Reboot done
[INFO]: Flash write success.
```

## Device Connection and Interaction

### Device Connection

After flashing the firmware, the development board needs to connect to the network and register with the cloud to enable remote communication. Once provisioned, three-way communication between `Phone` - `Tuya Cloud` - `Device` is established.

Device provisioning guide: [Device Network Configuration](https://tuyaopen.ai/en/docs/quick-start/device-network-configuration)

### Device Interaction

After successful provisioning, you can control the switch DP via the Tuya Smart APP, and the LED on the development board will turn on or off accordingly. You can also open the Arduino IDE Serial Monitor (baud rate 115200) to view device runtime logs.

#### LED Behavior

| State | LED |
| :---: | --- |
| APP sends switch `true` | LED turns on (GPIO low level) |
| APP sends switch `false` | LED turns off (GPIO high level) |

## Example Code Explanation

### Hardware Configuration

```cpp
#define APP_LED_PIN  1   // LED pin number (GPIO 1)
```

### IoT Initialization

```cpp
// Set IoT event callback function
TuyaIoT.setEventCallback(tuyaIoTEventCallback);

// Set device authorization code (tries to read from onboard storage first; falls back to macro-defined values on failure)
tuya_iot_license_t license;
int rt = TuyaIoT.readBoardLicense(&license);
if (OPRT_OK != rt) {
    license.uuid = (char *)TUYA_DEVICE_UUID;
    license.authkey = (char *)TUYA_DEVICE_AUTHKEY;
}
TuyaIoT.setLicense(license.uuid, license.authkey);

// Start IoT service
TuyaIoT.begin(TUYA_PRODUCT_ID, PROJECT_VERSION);
```

> This example uses `TuyaIoT.readBoardLicense()` to attempt reading the authorization code from onboard storage. If the read fails, it falls back to the default values defined in the macros.

### IoT Event Handling

All IoT platform events are handled uniformly in `tuyaIoTEventCallback`. The main events include:

| Event | Description |
| --- | --- |
| `TUYA_EVENT_BIND_START` | Device starts network provisioning |
| `TUYA_EVENT_MQTT_CONNECTED` | MQTT connection successful, device goes online |
| `TUYA_EVENT_TIMESTAMP_SYNC` | Cloud time synchronization |
| `TUYA_EVENT_RESET` | Device has been reset |
| `TUYA_EVENT_UPGRADE_NOTIFY` | OTA upgrade notification received |
| `TUYA_EVENT_DP_RECEIVE_OBJ` | Object-type DP data received |
| `TUYA_EVENT_DP_RECEIVE_RAW` | Raw-type DP data received |

### DP Data Interaction

Tuya IoT uses **DP (Data Point)** to exchange data between the cloud and the device. This example demonstrates the complete handling flow for object-type DPs:

#### Receiving DP Data

When the APP sends a command, the device receives DP data through the `TUYA_EVENT_DP_RECEIVE_OBJ` event and handles it based on the DP type:

| DP Type | Macro | Description |
| --- | --- | --- |
| Boolean | `PROP_BOOL` | Switch value, e.g., LED on/off |
| Integer | `PROP_VALUE` | Numeric value, e.g., brightness, temperature |
| String | `PROP_STR` | Text data |
| Enum | `PROP_ENUM` | Limited options, e.g., mode selection |
| Bitmap | `PROP_BITMAP` | Bit flag data |

Core handling logic:

```cpp
case PROP_BOOL:
    if (dp->value.dp_bool == true) {
        digitalWrite(APP_LED_PIN, LOW);   // LED on
    } else {
        digitalWrite(APP_LED_PIN, HIGH);  // LED off
    }
    break;
```

#### Reporting DP Data

After processing a DP, you must call `TuyaIoT.write()` to report the state back to the cloud, keeping the APP and device state synchronized:

```cpp
// Report object-type DP
TuyaIoT.write((dpobj->dps->id), (dpobj->dps->value), 0);

// Report raw-type DP
TuyaIoT.write((dpraw->dp.id), (dpraw->dp.data), (dpraw->dp.len), 3);
```

## Related Documentation

- [TuyaOpen Official Website](https://tuyaopen.ai)
- [Tuya Cloud Product Creation](https://tuyaopen.ai/en/docs/cloud/tuya-cloud/creating-new-product)
- [GitHub Repository](https://github.com/tuya/TuyaOpen)

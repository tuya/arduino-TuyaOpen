# WiFi Client Connect - Connection Management Example

## Overview

This example demonstrates advanced WiFi connection management, including connection status monitoring, timeout handling, auto-reconnect control, and disconnect functionality. It shows best practices for handling WiFi connections in production applications.

## Features

- Detailed WiFi connection status monitoring
- Connection attempt timeout (10 seconds)
- Connection state machine
- Auto-reconnect control
- Manual disconnect capability
- Comprehensive error handling
- Status reporting for all connection states

## Hardware Requirements

- Tuya IoT development board with WiFi capability
- WiFi router/access point
- Optional: Button on GPIO 8 for manual disconnect

## Configuration

Update WiFi credentials:

```cpp
const char* ssid = "your_wifi_ssid";
const char* password = "your_wifi_password";
```

## Connection States

The example monitors and reports the following WiFi states:

- `WSS_IDLE` - Not connected
- `WSS_NO_AP_FOUND` - SSID not found
- `WSS_CONN_FAIL` - Connection failed
- `WSS_PASSWD_WRONG` - Password incorrect
- `WSS_CONN_SUCCESS` - Connection successful (scan completed)
- `WSS_GOT_IP` - IP address obtained
- `WSS_DHCP_FAIL` - DHCP failed

## How It Works

### Setup Phase

1. **Initialization**
   - Configure serial communication
   - Set up GPIO button input
   - Begin WiFi connection

2. **Connection Attempt**
   - Try to connect with 500ms intervals
   - Maximum 20 attempts (10 seconds total)
   - Monitor status changes

3. **Status Handling**
   - Report each status change
   - Handle connection failures
   - Display IP on success
   - Force disconnect on timeout

### Loop Phase

- Monitor button state
- Disconnect WiFi when button pressed
- Can be expanded for reconnection logic

## Code Structure

```cpp
void setup() {
    // Serial initialization
    // Button GPIO setup
    // Start WiFi connection
    // Monitor connection status with timeout
    // Handle each status case
}

void loop() {
    // Monitor button
    // Handle disconnect
    // Can add reconnection logic
}
```

## Auto-Reconnect Control

By default, auto-reconnect is enabled. To disable:

```cpp
WiFi.setAutoReconnect(false);
```

**Enabled (default):**
- WiFi automatically reconnects on disconnect
- No manual intervention needed
- Suitable for most applications

**Disabled:**
- Manual reconnect required
- More control over connection lifecycle
- Useful for power-sensitive applications

## Usage

### Basic Usage
1. Update WiFi credentials
2. Upload to board
3. Open Serial Monitor (115200 baud)
4. Observe connection process
5. View detailed status updates

### With Button Control
1. Connect button to GPIO 8
2. Run the example
3. Press button to disconnect
4. Observe disconnect behavior

## Expected Output

**Successful Connection:**
```
[WiFi] Connecting to YourSSID
[WiFi] Scan is completed
[WiFi] WiFi is connected!
[WiFi] IP address: 192.168.1.105
```

**SSID Not Found:**
```
[WiFi] Connecting to YourSSID
[WiFi] SSID not found
[WiFi] WiFi Status: 3
[WiFi] Failed to connect to WiFi!
```

**Connection Timeout:**
```
[WiFi] Connecting to YourSSID
[WiFi] WiFi Status: 0
...
[WiFi] Failed to connect to WiFi!
```

## Troubleshooting

**SSID Not Found:**
- Verify SSID spelling
- Check WiFi is broadcasting SSID
- Ensure 2.4GHz band (if applicable)
- Move closer to router

**Connection Failed:**
- Check password
- Verify security type compatibility
- Check router settings
- Look for MAC address filtering

**IP Not Obtained:**
- Check DHCP server
- Verify network configuration
- Check router DHCP pool
- Try static IP configuration

**Constant Reconnection:**
- Check signal strength
- Look for interference
- Verify router stability
- Check power supply

## Advanced Features

### Manual Reconnection

```cpp
void loop() {
    if(WiFi.status() != WSS_GOT_IP) {
        Serial.println("Reconnecting...");
        WiFi.begin(ssid, password);
    }
    delay(10000);
}
```

### Connection Timeout Adjustment

```cpp
int tryDelay = 1000;      // 1 second between tries
int numberOfTries = 30;    // 30 tries = 30 seconds total
```

### Status Callback

```cpp
void WiFiEvent(WiFiEvent_t event) {
    // Handle WiFi events
}
WiFi.onEvent(WiFiEvent);
```

## Best Practices

1. **Always implement timeout** - Prevent indefinite hanging
2. **Monitor connection status** - React to state changes
3. **Handle all error cases** - Provide user feedback
4. **Use auto-reconnect wisely** - Consider power consumption
5. **Log status changes** - Aid in debugging

## Notes

- Connection attempts use 500ms intervals
- 10-second total timeout (20 × 500ms)
- Button state is checked in loop
- Disconnect is manual, reconnect can be automatic
- Core Debug Level "Verbose" provides more details

## Related Examples

- WiFiClientEvents - Comprehensive event handling
- WiFiClientBasic - Simple client connection
- WiFiMulti - Multiple AP support
- WiFiScan - Network scanning

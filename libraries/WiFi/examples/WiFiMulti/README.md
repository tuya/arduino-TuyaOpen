# WiFi Multi - Multiple Access Point Management

## Overview

This example demonstrates how to configure the device to automatically connect to the strongest available WiFi network from a list of multiple access points. The WiFiMulti library intelligently manages connections to multiple SSIDs, automatically selecting and connecting to the best available network based on signal strength. This is ideal for mobile devices or applications that operate in areas with multiple WiFi networks.

## Features

- **Multiple AP Support**: Configure connections to multiple WiFi networks
- **Automatic Selection**: Automatically connects to the strongest available network
- **Seamless Switching**: Automatically switches networks if current connection degrades
- **Connection Management**: Maintains connection and reconnects if disconnected
- **Signal-Based Priority**: Selects network with best RSSI (signal strength)

## Hardware Requirements

- Tuya Open development board with WiFi capability
- USB cable for programming and serial monitoring
- Access to multiple WiFi networks (or one network for testing)

## Configuration

Add your WiFi network credentials to the sketch:

```cpp
wifiMulti.addAP("ssid_from_AP_1", "your_password_for_AP_1");
wifiMulti.addAP("ssid_from_AP_2", "your_password_for_AP_2");
wifiMulti.addAP("ssid_from_AP_3", "your_password_for_AP_3");
```

You can add as many networks as needed. The library will attempt to connect to each in order of signal strength.

## How It Works

### WiFiMulti Library

The WiFiMulti class manages multiple WiFi credentials and handles:

1. **Network Scanning**: Periodically scans for available networks
2. **Signal Evaluation**: Compares signal strength (RSSI) of available networks
3. **Best Selection**: Connects to the strongest available network
4. **Auto-Reconnect**: Maintains connection and reconnects if dropped
5. **Auto-Switch**: Switches to better network if current degrades

### Connection Process

```cpp
// Add networks
wifiMulti.addAP("Network1", "password1");
wifiMulti.addAP("Network2", "password2");

// Attempt connection
if(wifiMulti.run() == WSS_GOT_IP) {
    // Connected successfully
}
```

### Connection Status

The `wifiMulti.run()` function returns WiFi status:
- `WSS_GOT_IP`: Successfully connected with IP assigned
- Other values: Not connected or connection in progress

### Continuous Monitoring

Call `wifiMulti.run()` periodically in the loop to:
- Maintain current connection
- Detect disconnections
- Switch to better network if available
- Reconnect automatically

## Usage

1. **Add Networks**: Configure multiple WiFi network credentials
2. **Upload Sketch**: Upload to your Tuya Open board
3. **Open Serial Monitor**: Set baud rate to 115200
4. **Observe Connection**: Watch connection status and selected network

### Expected Output

**Successful Connection:**
```
Connecting Wifi...

WiFi connected
IP address: 
192.168.1.100
```

**Connection Monitoring:**
```
WiFi not connected!
WiFi not connected!
WiFi connected
IP address: 
192.168.1.100
```

## Connection Priority

### Signal-Based Selection

WiFiMulti selects networks based on:

1. **Availability**: Network must be in range and broadcasting
2. **Signal Strength**: Higher RSSI (less negative) = higher priority
3. **Order Added**: If signals are similar, earlier added networks preferred

### Example Scenario

```
Available Networks:
- HomeWiFi:    RSSI -45 dBm (Excellent)
- OfficeWiFi:  RSSI -65 dBm (Good)
- GuestWiFi:   RSSI -80 dBm (Fair)

Selected: HomeWiFi (strongest signal)
```

### Network Switching

The library may switch networks if:
- Current connection is lost
- A significantly stronger network becomes available
- Current signal degrades below acceptable level

## Use Cases

- **Mobile Devices**: Robots or vehicles moving between coverage areas
- **Multi-Floor Buildings**: Different APs on different floors
- **Backup Networks**: Primary and fallback network configuration
- **Public Spaces**: Multiple WiFi options in malls, airports, etc.
- **Home and Office**: Seamless transition between locations
- **Guest Networks**: Fallback to guest network if main unavailable

## Advanced Configuration

### Adding Networks Dynamically

```cpp
// Can add networks at any time
void addNewNetwork(const char* ssid, const char* password) {
    wifiMulti.addAP(ssid, password);
}
```

### Checking Current Connection

```cpp
if(WiFi.status() == WSS_GOT_IP) {
    Serial.println("Connected to: " + WiFi.SSID());
    Serial.println("IP: " + WiFi.localIP().toString());
    Serial.println("RSSI: " + String(WiFi.RSSI()) + " dBm");
}
```

### Fast Connection

```cpp
// Set shorter connection timeout
wifiMulti.run(5000);  // 5 second timeout
```

## Troubleshooting

### Cannot Connect to Any Network

- Verify at least one network is in range
- Check that credentials are correct for all networks
- Ensure WiFi is enabled and antenna connected
- Look for typos in SSID or password

### Frequent Disconnections

- Networks may be at edge of range
- Check for interference or congestion
- Verify router stability
- Consider adding closer access points

### Connects to Wrong Network

- Library prioritizes signal strength
- Cannot manually set preference order
- Consider using single-network connection if specific AP needed

### Slow to Connect

- WiFiMulti scans all configured networks
- Reduce number of configured networks if possible
- First connection may take longer than subsequent ones

### Not Switching to Better Network

- May require significant RSSI difference to trigger switch
- Current connection stable connections preferred over slightly better alternatives
- Forced reconnection requires calling WiFi.disconnect()

## Best Practices

1. **Network Count**: Limit to 3-5 networks for faster scanning
2. **Credentials Security**: Store credentials securely, not in code
3. **Regular Monitoring**: Call `wifiMulti.run()` at least every few seconds
4. **Connection Validation**: Always check return value of `run()`
5. **Logging**: Log which network is connected for debugging

## Code Structure

### Setup Phase

```cpp
void setup() {
    Serial.begin(115200);
    
    // Add all networks
    wifiMulti.addAP("Network1", "pass1");
    wifiMulti.addAP("Network2", "pass2");
    
    // Initial connection
    Serial.println("Connecting Wifi...");
    if(wifiMulti.run() == WSS_GOT_IP) {
        Serial.println("WiFi connected");
        Serial.println("IP: " + WiFi.localIP().toString());
    }
}
```

### Loop Phase

```cpp
void loop() {
    // Maintain connection
    if(wifiMulti.run() != WSS_GOT_IP) {
        Serial.println("WiFi not connected!");
        delay(1000);
    } else {
        // Do work when connected
    }
}
```

## Notes

- WiFiMulti handles all connection management automatically
- No need to call `WiFi.begin()` when using WiFiMulti
- Library continuously monitors signal strength
- Reconnection is automatic on disconnection
- All configured networks should use same WiFi mode (STA)

## Related Examples

- WiFiClient - Basic single network connection
- WiFiScan - Manual network scanning
- WiFiClientEvents - Monitor connection events
- WiFiClientStaticIP - Static IP configuration

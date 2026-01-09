# WiFi Scan Example

## Overview

This example demonstrates how to scan for available WiFi networks in the vicinity. It performs periodic scans and displays detailed information about each detected network, including SSID, signal strength (RSSI), channel, and encryption type. This is useful for network discovery, site surveys, and choosing the best network to connect to.

## Features

- **Network Discovery**: Scans for all available WiFi networks in range
- **Detailed Information**: Displays SSID, RSSI, channel, and encryption type
- **Periodic Scanning**: Continuously scans at regular intervals
- **Encryption Detection**: Identifies security protocols (Open, WEP, WPA, WPA2, WPA3)
- **Memory Management**: Properly cleans up scan results to free memory

## Hardware Requirements

- Tuya Open development board with WiFi capability
- USB cable for programming and serial monitoring

## Configuration

No WiFi credentials are needed for this example. The sketch operates in station mode but doesn't connect to any network.

## How It Works

### Scan Process

1. **WiFi Initialization**: Set WiFi to station mode without connecting
2. **Network Scanning**: Call `WiFi.scanNetworks()` to scan for available networks
3. **Result Processing**: Iterate through scan results and display information
4. **Memory Cleanup**: Delete scan results to free memory
5. **Repeat**: Wait 5 seconds and scan again

### WiFi Modes

```cpp
WiFi.mode(WIFI_STA);    // Set to station mode
WiFi.disconnect();       // Ensure not connected
```

### Scanning

```cpp
int n = WiFi.scanNetworks();
```

This function:
- Blocks until scan completes
- Returns number of networks found
- Returns 0 if no networks found
- Results accessible via WiFi.SSID(), WiFi.RSSI(), etc.

### Network Information Access

For each network found:

```cpp
WiFi.SSID(i)           // Network name
WiFi.RSSI(i)           // Signal strength in dBm
WiFi.channel(i)        // WiFi channel (1-14)
WiFi.encryptionType(i) // Security protocol
```

### Memory Management

```cpp
WiFi.scanDelete();  // Free memory used by scan results
```

## Usage

1. **Upload Sketch**: Upload to your Tuya Open board
2. **Open Serial Monitor**: Set baud rate to 115200
3. **Observe Scans**: Watch as networks are discovered every 5 seconds

### Expected Output

```
Setup done
Scan start
Scan done
5 networks found
Nr | SSID                             | RSSI | CH | Encryption
1  | 		HomeNetwork		 | 	-45		 | 	6		 | 	WPA2 PSK
2  | 		OfficeWiFi		 | 	-52		 | 	11		 | 	WPA + WPA2
3  | 		GuestNetwork		 | 	-67		 | 	1		 | 	WPA2 PSK
4  | 		PublicHotspot		 | 	-75		 | 	6		 | 	open
5  | 		Neighbor_2.4G		 | 	-82		 | 	3		 | 	WPA2 PSK

Scan start
...
```

## Understanding the Output

### RSSI (Signal Strength)

| RSSI Range | Quality | Description |
|------------|---------|-------------|
| -30 to -50 dBm | Excellent | Very strong signal |
| -50 to -60 dBm | Good | Strong, reliable connection |
| -60 to -70 dBm | Fair | Acceptable, may have issues |
| -70 to -80 dBm | Weak | Poor connection quality |
| -80 to -90 dBm | Very Weak | Unstable, frequent drops |
| Below -90 dBm | Unusable | Too weak to maintain connection |

### WiFi Channels

**2.4 GHz Band:**
- Channels 1-14 (channel 14 Japan only)
- Non-overlapping channels: 1, 6, 11
- Channel width: 20 MHz or 40 MHz

**5 GHz Band:**
- Channels vary by region
- More non-overlapping channels available
- Less interference

### Encryption Types

The example recognizes these security protocols:

- **WAAM_OPEN**: No encryption (public network)
- **WAAM_WEP**: WEP (obsolete, insecure)
- **WAAM_WPA_PSK**: WPA with pre-shared key
- **WAAM_WPA2_PSK**: WPA2 (most common)
- **WAAM_WPA_WPA2_PSK**: Mixed WPA/WPA2
- **WAAM_WPA_WPA3_SAE**: WPA3 (newest, most secure)
- **unknown**: Unrecognized encryption

## Use Cases

- **Network Discovery**: Find available WiFi networks
- **Site Survey**: Assess WiFi coverage in an area
- **Channel Analysis**: Identify congested channels
- **Signal Mapping**: Create signal strength maps
- **Best AP Selection**: Choose strongest network to connect
- **Network Monitoring**: Track nearby network changes
- **Security Audit**: Identify insecure networks (Open, WEP)
- **Interference Detection**: Find overlapping channels

## Advanced Usage

### Async Scanning

For non-blocking scans, use async mode:

```cpp
WiFi.scanNetworks(true);  // true = async mode

// Check if scan complete
int n = WiFi.scanComplete();
if (n >= 0) {
    // Scan done, n = number of networks
}
```

### Hidden Network Detection

```cpp
// Scan for hidden networks too
int n = WiFi.scanNetworks(false, true);  // false=sync, true=show hidden
```

### Filtering Results

```cpp
// Only show networks with good signal
for (int i = 0; i < n; ++i) {
    if (WiFi.RSSI(i) > -70) {
        Serial.println(WiFi.SSID(i));
    }
}
```

### Channel Occupancy Analysis

```cpp
int channelCount[14] = {0};
for (int i = 0; i < n; ++i) {
    int ch = WiFi.channel(i);
    if (ch >= 1 && ch <= 14) {
        channelCount[ch - 1]++;
    }
}
// Find least congested channel
```

## Troubleshooting

### No Networks Found

- Verify WiFi antenna is properly connected
- Check if WiFi is enabled in your area
- Ensure board is not in metal enclosure (shields RF)
- Try moving to different location

### Scan Takes Long Time

- Normal scan duration: 1-3 seconds per channel
- Scanning all channels takes longer
- Cannot speed up significantly (hardware limitation)

### Incomplete Network List

- Some networks may be hidden (not broadcasting SSID)
- Networks on 5 GHz band won't appear (if board only supports 2.4 GHz)
- Very weak signals may not be detected

### Duplicate Networks

- Same SSID with multiple APs will appear multiple times
- Each entry represents a different physical access point
- Check MAC address (BSSID) to distinguish

### RSSI Fluctuations

- Signal strength varies due to interference, movement, etc.
- Average multiple readings for accurate measurements
- Consider environmental factors (walls, objects, people)

## Performance Considerations

### Scan Frequency

- Don't scan too frequently (causes WiFi overhead)
- Recommended minimum interval: 5 seconds
- Scanning prevents normal WiFi communication

### Memory Usage

- Each scan result uses memory
- Always call `WiFi.scanDelete()` after processing
- Limit scan results if memory is constrained

### Power Consumption

- Scanning consumes significant power
- Consider longer intervals for battery-powered devices
- Use async scanning to allow other tasks

## Notes

- Scanning works without connecting to any network
- Station mode is required for scanning
- Scan results are valid until next scan or delete
- The example scans indefinitely in a loop
- RSSI values are always negative (closer to 0 = stronger)
- Channel numbers vary by country/region

## Related Examples

- WiFiMulti - Automatically connect to strongest network
- WiFiScanDualAntenna - Scan with dual antenna support
- WiFiClient - Connect to specific network
- WiFiClientEvents - Monitor WiFi events

# WiFi Scan Dual Antenna Example

## Overview

This example demonstrates how to perform WiFi network scanning using dual antenna configuration. It shows how to configure and use two antennas for improved WiFi reception and signal diversity. The dual antenna setup can automatically or manually select the best antenna for transmission and reception, improving overall WiFi performance and reliability.

## Features

- **Dual Antenna Configuration**: Configure two GPIO pins for antenna switching
- **Automatic Antenna Selection**: Let the system choose the best antenna
- **TX/RX Independent Control**: Separate antenna selection for transmit and receive
- **Network Scanning**: Scan for available WiFi networks with improved reception
- **Diversity Support**: Hardware antenna diversity for better signal quality

## Hardware Requirements

- Tuya Open development board with dual antenna support
- Two WiFi antennas (or antenna connectors)
- USB cable for programming and serial monitoring

**Note:** Not all boards support dual antenna configuration. Verify your board's capabilities before using this example.

## Configuration

### Antenna GPIO Pins

Configure the GPIO pins connected to your antenna switches:

```cpp
#define GPIO_ANT1 2   // GPIO for antenna 1
#define GPIO_ANT2 25  // GPIO for antenna 2
```

**Important:** These pin numbers must match your hardware design. Incorrect pins may damage the board or antennas.

### Antenna Selection Mode

The example uses automatic antenna selection for both TX and RX:

```cpp
WiFi.setDualAntennaConfig(GPIO_ANT1, GPIO_ANT2, 
                          WIFI_RX_ANT_AUTO,  // Auto select RX antenna
                          WIFI_TX_ANT_AUTO); // Auto select TX antenna
```

## How It Works

### Dual Antenna Configuration

The `setDualAntennaConfig()` function sets up antenna diversity:

```cpp
bool err = WiFi.setDualAntennaConfig(
    GPIO_ANT1,          // GPIO for antenna 1
    GPIO_ANT2,          // GPIO for antenna 2
    WIFI_RX_ANT_AUTO,   // RX antenna mode
    WIFI_TX_ANT_AUTO    // TX antenna mode
);
```

### Antenna Selection Modes

Available modes for RX and TX antennas:

- **WIFI_RX_ANT_AUTO** / **WIFI_TX_ANT_AUTO**: Automatic selection based on signal quality
- **WIFI_RX_ANT0** / **WIFI_TX_ANT0**: Force use of antenna 0 (ANT1)
- **WIFI_RX_ANT1** / **WIFI_TX_ANT1**: Force use of antenna 1 (ANT2)

### Antenna Diversity

Antenna diversity improves performance by:

1. **Signal Comparison**: Continuously monitors signal quality on both antennas
2. **Automatic Switching**: Switches to antenna with better signal
3. **Multipath Reduction**: Reduces interference from signal reflections
4. **Coverage Improvement**: Better coverage in different orientations

### Scanning Process

1. Set WiFi to station mode
2. Configure dual antenna setup
3. Disconnect from any network
4. Scan for available networks
5. Display results with RSSI
6. Repeat scan every 5 seconds

## Usage

1. **Verify Hardware**: Ensure your board supports dual antennas
2. **Configure GPIOs**: Update GPIO pin numbers to match your hardware
3. **Upload Sketch**: Upload to your Tuya Open board
4. **Open Serial Monitor**: Set baud rate to 115200
5. **Observe Results**: Watch antenna configuration status and scan results

### Expected Output

**Successful Configuration:**
```
Dual Antenna configuration successfully done!
Setup done
scan start
scan done
5 networks found
1: HomeNetwork (-42)*
2: OfficeWiFi (-58)*
3: GuestNetwork (-65) 
4: PublicHotspot (-73) 
5: Neighbor_2.4G (-85)*

scan start
...
```

**Configuration Failure:**
```
Dual Antenna configuration failed!
Setup done
...
```

## Antenna Selection Strategies

### Automatic Mode (Recommended)

```cpp
WiFi.setDualAntennaConfig(GPIO_ANT1, GPIO_ANT2, 
                          WIFI_RX_ANT_AUTO, 
                          WIFI_TX_ANT_AUTO);
```

**Advantages:**
- Best overall performance
- Adapts to changing conditions
- No manual intervention needed

**Use When:**
- Mobile or rotating devices
- Variable signal environments
- General purpose applications

### Fixed Primary Antenna

```cpp
WiFi.setDualAntennaConfig(GPIO_ANT1, GPIO_ANT2, 
                          WIFI_RX_ANT0,  // Use ANT1 for RX
                          WIFI_TX_ANT0); // Use ANT1 for TX
```

**Advantages:**
- Consistent behavior
- Lower power (no switching)
- Predictable performance

**Use When:**
- Known best antenna position
- Fixed installation
- Power-critical applications

### Split TX/RX Configuration

```cpp
WiFi.setDualAntennaConfig(GPIO_ANT1, GPIO_ANT2, 
                          WIFI_RX_ANT_AUTO,  // Auto for RX
                          WIFI_TX_ANT0);     // Fixed for TX
```

**Use When:**
- Different antenna characteristics
- Optimizing for specific use case
- One antenna has better TX performance

## Use Cases

- **Mobile Devices**: Robots, drones, vehicles with changing orientation
- **Harsh Environments**: Industrial settings with multipath interference
- **Extended Range**: Maximize coverage in large spaces
- **Directional Antennas**: Switch between omnidirectional and directional
- **Redundancy**: Backup antenna if primary fails
- **Signal Optimization**: Always use best available antenna

## Troubleshooting

### "Dual Antenna configuration failed!"

- Board may not support dual antenna feature
- GPIO pins may be incorrect
- Pins may already be in use by other peripherals
- Check board documentation for supported pins

### No Improvement in Signal

- Antennas may be too close together
- Both antennas in same orientation
- Insufficient diversity gain
- Antennas not properly connected

### Frequent Antenna Switching

- Normal in changing environments
- May indicate similar signal strength
- Can increase power consumption slightly
- Consider fixed antenna if problematic

### One Antenna Not Working

- Check GPIO pin configuration
- Verify physical antenna connection
- Test with fixed antenna mode to isolate issue
- Check for hardware damage

### Reduced Performance

- GPIO pins may conflict with other functions
- Switching overhead in some scenarios
- Try fixed antenna mode for comparison

## Hardware Considerations

### Antenna Placement

- **Separation**: Place antennas at least λ/4 apart (≈3 cm at 2.4 GHz)
- **Orientation**: Different orientations maximize diversity
- **Isolation**: Minimize coupling between antennas
- **Clearance**: Keep antennas away from metal objects

### Antenna Types

- **Omnidirectional**: Equal radiation in all directions (common)
- **Directional**: Focused beam in one direction (higher gain)
- **PCB Antenna**: Integrated on board
- **External**: Connected via U.FL/IPEX connector

### GPIO Requirements

- Must be dedicated GPIO pins
- Cannot share with other functions during WiFi operation
- Check board schematic for antenna switch circuit
- Typical implementation uses RF switches controlled by GPIOs

## Performance Comparison

| Configuration | Typical Gain | Power | Complexity |
|--------------|--------------|-------|------------|
| Single Antenna | 0 dB (baseline) | Lowest | Simple |
| Dual Auto | 3-5 dB | Medium | Moderate |
| Dual Fixed | 0-3 dB | Low | Simple |

## Advanced Configuration

### Dynamic Switching

```cpp
// Switch to antenna 0 for specific operation
WiFi.setDualAntennaConfig(GPIO_ANT1, GPIO_ANT2, 
                          WIFI_RX_ANT0, WIFI_TX_ANT0);

// Back to auto mode
WiFi.setDualAntennaConfig(GPIO_ANT1, GPIO_ANT2, 
                          WIFI_RX_ANT_AUTO, WIFI_TX_ANT_AUTO);
```

### Monitoring Performance

```cpp
// Compare scan results with different antenna configs
int rssi_auto = scanWithConfig(WIFI_RX_ANT_AUTO);
int rssi_ant0 = scanWithConfig(WIFI_RX_ANT0);
int rssi_ant1 = scanWithConfig(WIFI_RX_ANT1);

// Choose best configuration
```

## Notes

- Dual antenna configuration is board-specific
- Not all ESP32/chip variants support antenna diversity
- Configuration must be set before WiFi operations
- GPIO pins are specific to your hardware design
- Automatic mode provides best general performance
- Power consumption slightly higher with diversity enabled
- The example displays `*` for encrypted networks

## Related Examples

- WiFiScan - Basic WiFi scanning without dual antenna
- WiFiClient - Connect using dual antenna configuration
- WiFiMulti - Multi-network connection with dual antenna

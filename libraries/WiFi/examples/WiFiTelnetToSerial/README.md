# WiFi Telnet to Serial Bridge

## Overview

This example creates a bidirectional bridge between Telnet (network) and Serial (UART). It allows you to connect to the device remotely via Telnet and interact with the serial console, essentially creating a wireless serial terminal. This is extremely useful for remote debugging, monitoring, and controlling devices without physical connection.

## Features

- **Telnet Server**: Accepts incoming Telnet connections on port 23
- **Serial Bridge**: Forwards data between Telnet client and serial port
- **Multiple Network Support**: Uses WiFiMulti for automatic best network selection
- **Bidirectional Communication**: Data flows both ways (Telnet ↔ Serial)
- **Client Management**: Handles client connections and disconnections
- **Connection Monitoring**: Automatically maintains WiFi connection

## Hardware Requirements

- Tuya Open development board with WiFi capability
- USB cable for programming and initial setup
- Serial device connected to UART (optional, for demonstration)

## Configuration

### WiFi Credentials

Configure your WiFi networks:

```cpp
const char* ssid     = "********";
const char* password = "********";

wifiMulti.addAP(ssid, password);
wifiMulti.addAP("ssid_from_AP_2", "your_password_for_AP_2");
wifiMulti.addAP("ssid_from_AP_3", "your_password_for_AP_3");
```

### Server Settings

```cpp
#define MAX_SRV_CLIENTS 1  // Maximum simultaneous Telnet clients
WiFiServer server(23);      // Telnet port (default: 23)
```

Adjust `MAX_SRV_CLIENTS` to allow more simultaneous connections if needed (increases memory usage).

## How It Works

### System Architecture

```
Telnet Client ←→ WiFi ←→ ESP32 ←→ UART ←→ Serial Device
     (PC)                (Bridge)              (Optional)
```

### Data Flow

**From Telnet to Serial:**
1. Telnet client sends data
2. WiFi receives data
3. Bridge reads from WiFi client
4. Bridge writes to Serial port

**From Serial to Telnet:**
1. Serial port receives data
2. Bridge reads from Serial
3. Bridge writes to all connected Telnet clients
4. WiFi transmits data to clients

### Connection Process

1. **WiFi Connection**: Connect to strongest available network
2. **Server Start**: Start Telnet server on port 23
3. **Client Acceptance**: Accept incoming Telnet connections
4. **Data Forwarding**: Continuously forward data bidirectionally
5. **Connection Maintenance**: Monitor and manage client connections

### Client Management

```cpp
// Accept new clients
if (server.hasClient()) {
    // Find free slot
    for(i = 0; i < MAX_SRV_CLIENTS; i++) {
        if (!serverClients[i] || !serverClients[i].connected()) {
            serverClients[i] = server.available();
            break;
        }
    }
}
```

## Usage

### Initial Setup

1. **Configure WiFi**: Update WiFi credentials
2. **Upload Sketch**: Upload to your Tuya Open board
3. **Open Serial Monitor**: Note the displayed IP address
4. **Connect via Telnet**: Use the IP address to connect

### Connecting with Telnet

**Windows (PowerShell/CMD):**
```
telnet 192.168.1.100 23
```

**Linux/macOS:**
```bash
telnet 192.168.1.100 23
```

**Alternative (netcat):**
```bash
nc 192.168.1.100 23
```

**PuTTY (Windows GUI):**
- Host: 192.168.1.100
- Port: 23
- Connection Type: Telnet

### Expected Output

**Serial Monitor:**
```
Connecting Wifi 
10
9
8
WiFi connected 
IP address: 192.168.1.100
Ready! Use 'telnet 192.168.1.100 23' to connect

New client: 0 192.168.1.50
```

**Telnet Client:**
```
Connected to 192.168.1.100
Escape character is '^]'.
[Everything typed here appears in Serial Monitor]
[Everything from Serial appears here]
```

## Use Cases

- **Remote Debugging**: Debug devices without physical access
- **Wireless Console**: Access serial console over network
- **Remote Monitoring**: Monitor serial output remotely
- **Device Configuration**: Configure devices via network instead of USB
- **Data Logging**: Collect serial data over network
- **Multi-User Access**: Multiple users can monitor same device (with MAX_SRV_CLIENTS > 1)
- **Industrial Automation**: Remote access to equipment serial interfaces
- **IoT Development**: Wireless serial access during development

## Advanced Configuration

### Change Telnet Port

```cpp
WiFiServer server(2323);  // Use port 2323 instead of 23
```

**Note:** Standard Telnet port is 23. Custom ports require specifying port in client.

### Multiple Client Support

```cpp
#define MAX_SRV_CLIENTS 4  // Allow up to 4 simultaneous clients
```

**Trade-off:** More clients = more memory usage and processing overhead.

### Disable Nagle Algorithm

```cpp
server.setNoDelay(true);  // Send data immediately, reduce latency
```

This reduces latency but may increase network overhead for small packets.

### Custom Serial Baud Rate

```cpp
Serial.begin(9600);   // Match baud rate of your serial device
Serial.begin(230400); // High-speed serial
```

### Timeout Configuration

Add timeout for idle clients:

```cpp
if (serverClients[i].connected()) {
    if (millis() - lastActivity[i] > 300000) {  // 5 minutes
        serverClients[i].stop();  // Disconnect idle client
    }
}
```

## Security Considerations

⚠️ **Important Security Warning:**

- Telnet sends data **unencrypted** (including passwords)
- Anyone on the network can intercept communication
- No authentication by default

### Security Recommendations

1. **Use on Trusted Networks Only**: Never expose on public networks
2. **Add Authentication**: Implement login/password check
3. **Use SSH Instead**: Consider ESP32 SSH libraries for encryption
4. **Firewall Rules**: Restrict access to specific IP addresses
5. **VPN**: Use VPN for remote access
6. **Change Default Port**: Use non-standard port (security through obscurity)

### Simple Authentication Example

```cpp
bool authenticated = false;
String password = "secret123";

// In client data handling:
if (!authenticated) {
    // Require password before allowing access
    if (receivedData == password) {
        authenticated = true;
        client.println("Authenticated!");
    } else {
        client.stop();
    }
}
```

## Troubleshooting

### Cannot Connect to Telnet Server

- Verify device is connected to WiFi (check serial output)
- Confirm IP address is correct
- Check firewall isn't blocking port 23
- Ensure Telnet client is installed (may need to enable on Windows)
- Try ping to verify network connectivity

### Connection Immediately Closes

- Server may have reached MAX_SRV_CLIENTS limit
- Check for client slot availability in serial output
- Increase MAX_SRV_CLIENTS if needed

### Data Not Appearing

- Check serial baud rate matches (115200 in this example)
- Verify data is actually being sent
- Try sending from both directions to isolate issue
- Check for buffer overflow on high data rates

### Garbled Characters

- Baud rate mismatch between devices
- Serial configuration mismatch (bits, parity, stop bits)
- Electrical noise on serial lines

### WiFi Keeps Disconnecting

- Signal strength too weak
- Network instability
- Power supply issues
- Check WiFi credentials

### High Latency

- Network congestion
- Try `server.setNoDelay(true)`
- Reduce WiFi distance
- Check for interference

## Performance Considerations

### Data Rate Limitations

- Serial: Up to 115200 baud (≈11.5 KB/s) typical
- WiFi: Much faster, not the bottleneck
- Practical limit depends on Serial baud rate

### Buffer Management

- Serial buffer: Default size varies (typically 64-128 bytes)
- WiFi buffer: Managed by library
- High data rates may require buffer size adjustments

### Multiple Clients

Each additional client:
- Increases memory usage
- Adds processing overhead
- May reduce maximum data rate

## Code Flow

```
loop() {
    ├─ Check WiFi connection
    ├─ Accept new Telnet clients
    │   └─ Find free client slot
    ├─ Check Telnet clients for data
    │   ├─ Read from Telnet
    │   └─ Write to Serial
    ├─ Check Serial for data
    │   ├─ Read from Serial
    │   └─ Write to all Telnet clients
    └─ Disconnect dead clients
}
```

## Notes

- Uses WiFiMulti for robust network connectivity
- Port 23 is standard Telnet port (may require admin rights on some systems)
- Serial baud rate is 115200 by default
- The example uses `tal_malloc` and `tal_free` for dynamic memory
- No authentication implemented - add security for production use
- Compatible with standard Telnet clients
- Consider SSH for encrypted communication in production

## Related Examples

- SimpleWiFiServer - Basic HTTP server
- WiFiClient - Basic WiFi client connection
- WiFiMulti - Multiple network management
- WiFiClientStaticIP - Use static IP for server

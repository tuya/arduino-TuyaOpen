# WiFi UDP Client Example

## Overview

This example demonstrates how to create a UDP (User Datagram Protocol) client that receives and responds to UDP packets over WiFi. Unlike TCP, UDP is a connectionless protocol that's ideal for applications requiring low latency, broadcast/multicast communication, or where some packet loss is acceptable. The example receives UDP packets, displays their content, and sends acknowledgment responses.

## Features

- **UDP Communication**: Connectionless packet-based communication
- **Packet Reception**: Receives and parses incoming UDP packets
- **Automatic Response**: Sends acknowledgment to packet sender
- **Remote Info Display**: Shows sender's IP address and port
- **Low Latency**: Minimal overhead compared to TCP
- **WiFi Integration**: Connects to WiFi network before starting UDP

## Hardware Requirements

- Tuya Open development board with WiFi capability
- USB cable for programming and serial monitoring

## Configuration

### WiFi Credentials

```cpp
const char * networkName = "********";  // Your WiFi network name
const char * networkPswd = "********";  // Your WiFi password
```

### UDP Settings

```cpp
const int udpPort = 4556;  // Local UDP port to listen on
```

You can change this to any available port (1024-65535 recommended for user applications).

## How It Works

### UDP vs TCP

| Feature | TCP | UDP |
|---------|-----|-----|
| Connection | Connection-oriented | Connectionless |
| Reliability | Guaranteed delivery | Best-effort delivery |
| Ordering | Packets ordered | No ordering |
| Speed | Slower (overhead) | Faster (minimal overhead) |
| Use Case | File transfer, web | Streaming, gaming, IoT |

### Communication Flow

1. **Connect to WiFi**: Establish WiFi connection
2. **Start UDP**: Begin listening on specified port
3. **Wait for Packets**: Block until packet received
4. **Parse Packet**: Extract data and sender information
5. **Display Content**: Show packet contents on serial
6. **Send Reply**: Send acknowledgment back to sender

### Packet Structure

Each received packet contains:
- **Remote IP**: IP address of sender
- **Remote Port**: Port number of sender
- **Packet Data**: The actual message content
- **Packet Size**: Length of received data

### Code Structure

```cpp
// Wait for packet
int packetSize = udp.parsePacket();

if (packetSize) {
    // Get sender info
    IPAddress remoteIP = udp.remoteIP();
    int remotePort = udp.remotePort();
    
    // Read packet data
    int n = udp.read(buffer, MAX_SIZE);
    
    // Send reply
    udp.beginPacket(remoteIP, remotePort);
    udp.write(replyData, replyLength);
    udp.endPacket();
}
```

## Usage

### Setup and Connection

1. **Configure WiFi**: Update network credentials
2. **Upload Sketch**: Upload to your Tuya Open board
3. **Open Serial Monitor**: Set baud rate to 115200
4. **Note IP Address**: Record the displayed IP address

### Expected Output

```
Connecting to YourNetwork
........
WiFi connected.
IP address: 
192.168.1.100

packet size: 13
remoteIP: 192.168.1.50
remotePort: 52341
Contents:
Hello ESP32!
```

### Sending UDP Packets

**Using netcat (Linux/macOS):**
```bash
echo "Hello ESP32!" | nc -u 192.168.1.100 4556
```

**Using Python:**
```python
import socket

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.sendto(b"Hello ESP32!", ("192.168.1.100", 4556))

# Receive acknowledgment
data, addr = sock.recvfrom(1024)
print(f"Received: {data.decode()}")
sock.close()
```

**Using Node.js:**
```javascript
const dgram = require('dgram');
const client = dgram.createSocket('udp4');

client.send('Hello ESP32!', 4556, '192.168.1.100', (err) => {
    if (err) console.error(err);
    
    client.on('message', (msg, rinfo) => {
        console.log(`Received: ${msg}`);
        client.close();
    });
});
```

**Using PowerShell (Windows):**
```powershell
$udpClient = New-Object System.Net.Sockets.UdpClient
$bytes = [System.Text.Encoding]::ASCII.GetBytes("Hello ESP32!")
$udpClient.Send($bytes, $bytes.length, "192.168.1.100", 4556)
$udpClient.Close()
```

## Use Cases

- **IoT Sensor Networks**: Lightweight sensor data transmission
- **Real-Time Monitoring**: Low-latency status updates
- **Broadcasting**: Send data to multiple devices
- **Gaming Applications**: Fast-paced game state updates
- **Media Streaming**: Audio/video streaming protocols
- **Discovery Protocols**: Device discovery and announcement
- **Logging**: Remote logging with acceptable packet loss
- **Command & Control**: Simple device control commands

## Advanced Features

### Broadcast Communication

To receive broadcast packets:

```cpp
// Allow broadcast reception
udp.begin(WiFi.localIP(), udpPort);

// On sender side (broadcasts to all devices):
udp.beginPacket(IPAddress(255, 255, 255, 255), udpPort);
```

### Multicast Support

```cpp
IPAddress multicastIP(239, 1, 2, 3);
udp.beginMulticast(multicastIP, udpPort);
```

### Non-Blocking Reception

```cpp
// Check if packet available without blocking
int packetSize = udp.parsePacket();
if (packetSize == 0) {
    // No packet, do other work
    return;
}
```

### Large Packet Handling

```cpp
#define UDP_TX_MAX_SIZE 1472  // Max UDP payload (typical)

if (packetSize > UDP_TX_MAX_SIZE) {
    // Handle oversized packet
    // May need fragmentation
}
```

### Packet Filtering

```cpp
// Only accept packets from specific IP
if (udp.remoteIP() == IPAddress(192, 168, 1, 50)) {
    // Process packet
} else {
    // Ignore packet
}
```

## Troubleshooting

### Not Receiving Packets

- Verify sender is using correct IP address
- Check port number matches (4556 by default)
- Ensure firewall isn't blocking UDP traffic
- Verify both devices on same network (or routed)
- Check with network sniffer (Wireshark)

### Packets Dropped

- Normal for UDP - no guaranteed delivery
- Network congestion may cause drops
- Check available memory (buffer overflow)
- Reduce packet rate if systematic drops occur

### Cannot Send Reply

- Check sender's firewall settings
- Sender may not be listening for reply
- Verify reply packet is properly formed
- Check network routing

### Wrong Data Received

- Check buffer size is sufficient
- Verify null termination: `buffer[n] = 0`
- Ensure character encoding matches
- Check for buffer overflow

### High Packet Loss

- Network congestion
- WiFi signal weak
- Sender transmitting too fast
- Receiver processing too slow (blocking code)

## Performance Considerations

### Packet Size

- **Maximum UDP Payload**: ~1472 bytes (typical)
- **Larger Packets**: May be fragmented by network
- **Optimal Size**: 512-1024 bytes for reliability
- **Fragmentation**: Increases loss probability

### Rate Limiting

```cpp
unsigned long lastPacketTime = 0;
const unsigned long minInterval = 10; // ms

if (millis() - lastPacketTime >= minInterval) {
    udp.beginPacket(ip, port);
    // Send packet
    lastPacketTime = millis();
}
```

### Memory Management

- Each packet uses RAM for buffering
- Reduce `UDP_TX_MAX_SIZE` if memory constrained
- Process packets quickly to free buffers

### Latency

- UDP typically < 1ms overhead
- Network latency dominates (typically 1-50ms LAN)
- WiFi adds variability
- Congestion can increase latency significantly

## UDP Best Practices

1. **Handle Packet Loss**: Implement application-level acknowledgments if needed
2. **Validate Data**: Always check packet size and content validity
3. **Null Terminate Strings**: Add null terminator when treating data as string
4. **Rate Limit**: Don't overwhelm network or receiver
5. **Use Checksums**: Add application-level checksums for critical data
6. **Timeout Handling**: Don't block indefinitely waiting for packets
7. **Buffer Management**: Check available space before sending

## Security Considerations

- UDP has no built-in encryption
- Anyone on network can intercept packets
- No authentication of sender
- Vulnerable to spoofing attacks

### Security Recommendations

1. **Encrypt Data**: Use application-level encryption
2. **Authenticate**: Verify sender identity
3. **Validate Input**: Never trust packet contents
4. **Rate Limit**: Prevent DoS attacks
5. **Firewall**: Restrict to necessary IP addresses

## Protocol Comparison

### When to Use UDP

✅ Use UDP when:
- Low latency is critical
- Occasional packet loss acceptable
- Broadcasting/multicasting needed
- Overhead must be minimal
- Real-time data (sensor readings, video, audio)

### When to Use TCP

✅ Use TCP when:
- Guaranteed delivery required
- Data must arrive in order
- Binary data or large transfers
- Connection state needed

## Notes

- UDP is connectionless - no handshake required
- Packets may arrive out of order
- No automatic retransmission on packet loss
- More efficient than TCP for many IoT applications
- The example uses port 4556 (can be changed)
- Serial baud rate is 115200
- Reply message is "acknowledged\r\n" (14 bytes)
- Buffer size is 1025 bytes (1024 + null terminator)

## Related Examples

- WiFiClient - TCP client connection
- SimpleWiFiServer - TCP server
- WiFiMulti - Multiple network management
- WiFiClientStaticIP - Static IP configuration

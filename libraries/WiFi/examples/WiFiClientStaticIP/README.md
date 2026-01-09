# WiFi Client Static IP Example

## Overview

This example demonstrates how to configure a WiFi connection using a static IP address instead of DHCP. Static IP addresses are useful in scenarios where consistent network addressing is required, such as in server applications, network automation, or when working with MAC address filtering and port forwarding.

## Features

- **Static IP Configuration**: Configure fixed IP address, gateway, subnet mask, and DNS server
- **Manual Network Settings**: Full control over all network parameters
- **HTTP Client Example**: Demonstrates using static IP to make HTTP requests
- **Network Information Display**: Shows all configured network parameters

## Hardware Requirements

- Tuya Open development board with WiFi capability
- USB cable for programming and serial monitoring
- Access to router settings (to ensure the static IP doesn't conflict with DHCP pool)

## Configuration

### WiFi Credentials

Update the WiFi network credentials:

```cpp
const char* ssid     = "********";  // Your WiFi network name
const char* password = "********";  // Your WiFi password
```

### Static IP Settings

Configure your static IP parameters:

```cpp
IPAddress local_IP(192, 168, 15, 69);     // Your desired static IP
IPAddress gateway(192, 168, 15, 1);       // Your router's IP
IPAddress subnet(255, 255, 255, 0);       // Subnet mask
IPAddress primaryDNS(8, 8, 8, 8);         // Google DNS (optional)
```

**Important:** Ensure your static IP:
- Is within your network's subnet range
- Does not conflict with your router's DHCP pool
- Matches your router's gateway address

### HTTP Request Target

For the HTTP client demonstration:

```cpp
const char* host = "example.com";
const char* url  = "/index.html";
```

## How It Works

### Static IP Configuration

The sketch configures static IP before connecting to WiFi:

```cpp
if (!WiFi.config(local_IP, gateway, subnet, primaryDNS)) {
    Serial.println("STA Failed to configure");
}
```

This must be called **before** `WiFi.begin()`.

### Connection Process

1. Configure static IP settings
2. Begin WiFi connection with credentials
3. Wait for connection establishment (WSS_GOT_IP status)
4. Display all network information
5. Periodically make HTTP requests to demonstrate connectivity

### Network Information Display

After successful connection, the sketch displays:
- IP address (your static IP)
- MAC address
- Subnet mask
- Gateway IP
- DNS server

## Usage

1. **Configure Network Settings**: Update the static IP configuration to match your network
2. **Verify Settings**: Ensure static IP doesn't conflict with DHCP
3. **Upload Sketch**: Upload to your Tuya Open board
4. **Open Serial Monitor**: Set baud rate to 115200
5. **Observe Connection**: Watch network info and HTTP responses

### Expected Output

```
Connecting to YourNetwork
.....
WiFi connected!
IP address: 192.168.15.69
Mac Address: AA:BB:CC:DD:EE:FF
Subnet Mask: 255.255.255.0
Gateway IP: 192.168.15.1
DNS: 8.8.8.8

connecting to example.com
Requesting URL: /index.html
HTTP/1.1 200 OK
...
closing connection
```

## Network Configuration Guidelines

### Choosing a Static IP

1. **Check Your Network Range**: Most home routers use 192.168.1.x or 192.168.0.x
2. **Avoid DHCP Pool**: Reserve a range outside your router's DHCP range
   - Example: If DHCP is 192.168.1.100-200, use 192.168.1.50
3. **Document Your IP**: Keep track of all static IPs to avoid conflicts

### Common Network Ranges

| Network Type | Gateway | Subnet Mask | IP Range |
|--------------|---------|-------------|----------|
| Class C (Home) | 192.168.1.1 | 255.255.255.0 | 192.168.1.2-254 |
| Class C Alt | 192.168.0.1 | 255.255.255.0 | 192.168.0.2-254 |
| Class C Custom | 10.0.0.1 | 255.255.255.0 | 10.0.0.2-254 |

### DNS Configuration

The example uses Google's public DNS (8.8.8.8). Alternatives:
- Google DNS: 8.8.8.8, 8.8.4.4
- Cloudflare: 1.1.1.1, 1.0.0.1
- Your router: Usually the gateway IP
- OpenDNS: 208.67.222.222, 208.67.220.220

## Use Cases

- **IoT Servers**: Devices that act as servers need consistent addresses
- **Port Forwarding**: Static IPs simplify port forwarding configuration
- **MAC Filtering**: Some networks require pre-registered IP/MAC pairs
- **Network Monitoring**: Easier to track devices with static IPs
- **Industrial Applications**: Fixed addressing for automation systems
- **Fast Boot**: Skip DHCP negotiation for faster connection

## Troubleshooting

### Cannot Connect to WiFi

- Verify all network parameters are correct
- Check that static IP is in valid range
- Ensure gateway IP matches your router
- Confirm subnet mask is correct (usually 255.255.255.0)

### "STA Failed to configure" Message

- Network parameters may be invalid
- Try calling `WiFi.config()` again
- Verify IPAddress objects are properly initialized

### HTTP Requests Fail

- Check DNS configuration is correct
- Try using router's IP as DNS instead of public DNS
- Verify internet connectivity from other devices
- Ensure firewall isn't blocking connections

### IP Conflict

- Another device may be using the same IP
- Change to a different static IP address
- Check router's connected devices list
- Use network scanning tools to find available IPs

### Cannot Reach Gateway

- Verify gateway IP matches router
- Check physical connection and signal strength
- Ensure router is functioning properly

## Advanced Configuration

### Using Router as DNS

```cpp
IPAddress primaryDNS(192, 168, 15, 1);  // Same as gateway
```

### Multiple DNS Servers

```cpp
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);
WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS);
```

### Dynamic to Static Switch

You can switch between DHCP and static during runtime:

```cpp
// Switch to static
WiFi.config(local_IP, gateway, subnet);

// Switch to DHCP
WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
```

## Notes

- Static IP configuration must be done before `WiFi.begin()`
- Configuration persists until changed or device reset
- DNS parameter is optional but recommended
- The example makes HTTP requests every 5 seconds
- Keep track of all static IPs in your network to avoid conflicts

## Related Examples

- WiFiClient - Basic WiFi connection with DHCP
- SimpleWiFiServer - Create a server with static IP
- WiFiClientEvents - Monitor network events

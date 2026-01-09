# WiFi Client Basic

## Overview

This is a basic WiFi TCP client example that demonstrates how to connect to a TCP server, send requests, and receive responses. It's perfect for learning the fundamentals of network communication with Arduino.

## Features

- WiFi station mode connection
- TCP client socket communication
- Send HTTP GET requests
- Receive and parse server responses
- Timeout handling
- Connection retry mechanism

## Hardware Requirements

- Tuya IoT development board with WiFi capability
- TCP server on your network (or use example.com)

## Configuration

Update the WiFi credentials and server details:

```cpp
WiFi.begin("your_ssid", "your_password");

const uint16_t port = 80;              // Server port
const char* host = "192.168.1.1";      // Server IP or hostname
```

## How It Works

### Connection Process

1. **WiFi Connection**
   - Connects to specified SSID
   - Waits for IP assignment
   - Verifies connection status

2. **TCP Connection**
   - Creates WiFiClient instance
   - Connects to server IP:port
   - Implements 5-second retry delay

3. **Data Exchange**
   - Sends HTTP GET request
   - Waits for server response (max 1 second)
   - Reads response line by line
   - Closes connection

## Code Structure

```cpp
void setup() {
    // Initialize serial
    // Connect to WiFi
    // Wait for connection
}

void loop() {
    // Create client
    // Connect to server
    // Send request
    // Read response
    // Close connection
    // Delay before retry
}
```

## Usage

1. Set up a TCP server on your network
2. Update WiFi credentials
3. Update host and port
4. Upload to board
5. Open Serial Monitor (115200 baud)
6. Observe connection and data exchange

## Example Servers

**Simple HTTP Server:**
```cpp
const char* host = "example.com";
const uint16_t port = 80;
client.print("GET /index.html HTTP/1.1\n\n");
```

**Custom TCP Server:**
```cpp
const char* host = "192.168.1.100";
const uint16_t port = 1337;
client.print("Your custom data");
```

## Expected Output

```
Waiting for WiFi... .....
WiFi connected
IP address: 192.168.1.105
Connecting to 192.168.1.1
HTTP/1.1 200 OK
Content-Type: text/html
...
Closing connection.
```

## Troubleshooting

**WiFi Not Connecting:**
- Verify SSID and password
- Check WiFi signal strength
- Ensure 2.4GHz network (if applicable)

**Server Connection Failed:**
- Verify server IP and port
- Check firewall settings
- Ensure server is running
- Test with ping command

**Response Timeout:**
- Server may be slow
- Network latency issues
- Increase timeout value
- Check server logs

## Advanced Usage

**Send Custom Data:**
```cpp
client.print("Your custom message");
```

**Read Multiple Lines:**
```cpp
while(client.available()) {
    String line = client.readStringUntil('\r');
    Serial.println(line);
}
```

**Check Connection State:**
```cpp
if(client.connected()) {
    // Still connected
}
```

## Notes

- WiFiClient is non-blocking
- Connection timeout prevents hanging
- Always close connections to free resources
- Suitable for HTTP, SMTP, or custom protocols

## Related Examples

- WiFiClient - ThingSpeak integration
- SimpleWiFiServer - TCP server
- WiFiClientStaticIP - Static IP configuration
- HTTPClient - Full HTTP client library

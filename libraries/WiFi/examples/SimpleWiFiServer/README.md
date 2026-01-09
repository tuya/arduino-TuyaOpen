# Simple WiFi Server Example

## Overview

This example demonstrates how to create a simple HTTP web server that controls hardware (LED) via web browser. The server hosts a basic web page with clickable links to turn an LED on and off. This is a fundamental example showing how to build IoT devices with web-based control interfaces accessible from any device with a web browser.

## Features

- **HTTP Web Server**: Hosts a web interface on port 80
- **LED Control**: Turn LED on/off via web clicks
- **Simple HTML Interface**: Basic but functional web page
- **GET Request Handling**: Responds to HTTP GET requests
- **Network Accessible**: Control from any device on the network
- **Real-Time Response**: Immediate hardware control

## Hardware Requirements

- Tuya Open development board with WiFi capability
- USB cable for programming and serial monitoring
- LED connected to GPIO 5 (or use onboard LED if available)

### LED Connection

```
GPIO 5 ----[LED]----[Resistor 220Ω]---- GND
```

**Note:** Many boards have a built-in LED that can be used instead.

## Configuration

### WiFi Credentials

```cpp
const char* ssid     = "********";  // Your WiFi network name
const char* password = "********";  // Your WiFi password
```

### Hardware Pin

```cpp
pinMode(5, OUTPUT);  // LED on GPIO 5
```

Change this if your LED is connected to a different pin.

### Server Port

```cpp
WiFiServer server(80);  // HTTP port (default: 80)
```

Port 80 is standard HTTP. You can use custom ports (e.g., 8080) if needed.

## How It Works

### HTTP Protocol Basics

The server implements a minimal HTTP server:

1. **Listen**: Wait for incoming TCP connections on port 80
2. **Accept**: Accept client connection (browser)
3. **Parse Request**: Read HTTP request line by line
4. **Process Commands**: Look for `/H` (LED on) or `/L` (LED off)
5. **Send Response**: Return HTML page with control links
6. **Close**: Disconnect client

### HTML Response

The server sends this HTML page:

```html
HTTP/1.1 200 OK
Content-type:text/html

Click <a href="/H">here</a> to turn the LED on pin 5 on.<br>
Click <a href="/L">here</a> to turn the LED on pin 5 off.<br>
```

### Request Handling

```cpp
if (currentLine.endsWith("GET /H")) {
    digitalWrite(5, HIGH);  // Turn LED on
}
if (currentLine.endsWith("GET /L")) {
    digitalWrite(5, LOW);   // Turn LED off
}
```

## Usage

### Setup and Connection

1. **Connect LED**: Wire LED to GPIO 5 (if not using onboard)
2. **Configure WiFi**: Update credentials in sketch
3. **Upload Sketch**: Upload to your Tuya Open board
4. **Note IP Address**: Open Serial Monitor and record IP
5. **Access Server**: Open browser and navigate to the IP

### Expected Output

**Serial Monitor:**
```
Connecting to YourNetwork
...........
WiFi connected.
IP address: 
192.168.1.100

New Client.
GET / HTTP/1.1
Host: 192.168.1.100
Connection: keep-alive
...
Client Disconnected.
```

**Web Browser:**

Navigate to `http://192.168.1.100`

You'll see:
```
Click here to turn the LED on pin 5 on.
Click here to turn the LED on pin 5 off.
```

Clicking the links controls the LED and refreshes the page.

## Web Interface Interaction

### Turning LED On

1. Click "here" next to "turn the LED on"
2. Browser sends: `GET /H HTTP/1.1`
3. Server receives request
4. Executes: `digitalWrite(5, HIGH)`
5. LED turns on
6. Server sends HTML response
7. Browser displays same page (LED now on)

### Turning LED Off

1. Click "here" next to "turn the LED off"
2. Browser sends: `GET /L HTTP/1.1`
3. Server receives request
4. Executes: `digitalWrite(5, LOW)`
5. LED turns off
6. Server sends HTML response

## Use Cases

- **IoT Device Control**: Web-based control of connected devices
- **Home Automation**: Simple light/appliance control
- **Remote Monitoring**: Control and monitor devices from browser
- **Prototyping**: Quick interface for testing hardware
- **Learning Platform**: Understand web server basics
- **Dashboard Creation**: Foundation for more complex web UIs
- **Network Diagnostics**: Verify network connectivity

## Expanding the Example

### Add Status Display

```cpp
client.print("LED Status: ");
client.print(digitalRead(5) ? "ON" : "OFF");
client.println("<br>");
```

### Multiple GPIO Control

```cpp
if (currentLine.endsWith("GET /LED1ON")) {
    digitalWrite(5, HIGH);
}
if (currentLine.endsWith("GET /LED2ON")) {
    digitalWrite(4, HIGH);
}
```

### Add CSS Styling

```cpp
client.println("<!DOCTYPE html><html><head>");
client.println("<style>");
client.println("body { font-family: Arial; margin: 50px; }");
client.println("a { padding: 10px; background: #4CAF50; color: white; text-decoration: none; }");
client.println("</style></head><body>");
client.println("<h1>LED Control</h1>");
client.println("<a href='/H'>Turn ON</a> ");
client.println("<a href='/L'>Turn OFF</a>");
client.println("</body></html>");
```

### JSON API

```cpp
if (currentLine.endsWith("GET /status")) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-type: application/json");
    client.println();
    client.print("{\"led\":\"");
    client.print(digitalRead(5) ? "on" : "off");
    client.println("\"}");
}
```

### Add Sensor Readings

```cpp
float temperature = readTemperature();
client.print("Temperature: ");
client.print(temperature);
client.println("°C<br>");
```

## Advanced Features

### URL Parameters

```cpp
// Parse URL: /led?state=on
if (currentLine.indexOf("GET /led?state=on") >= 0) {
    digitalWrite(5, HIGH);
}
```

### Form Handling (POST)

```cpp
if (currentLine.startsWith("POST /submit")) {
    // Read POST data
    // Parse form fields
}
```

### Multiple Pages

```cpp
if (currentLine.endsWith("GET /")) {
    // Home page
} else if (currentLine.endsWith("GET /settings")) {
    // Settings page
} else if (currentLine.endsWith("GET /about")) {
    // About page
}
```

### Authentication

```cpp
bool checkAuth(String request) {
    return request.indexOf("Authorization: Basic") >= 0;
}

if (!checkAuth(currentLine)) {
    client.println("HTTP/1.1 401 Unauthorized");
    client.println("WWW-Authenticate: Basic realm=\"ESP32\"");
    return;
}
```

## Troubleshooting

### Cannot Access Web Page

- Verify device connected to WiFi (check Serial Monitor)
- Confirm IP address is correct
- Ensure device and computer on same network
- Check firewall settings
- Try http:// explicitly (not https://)

### LED Not Responding

- Check LED is properly connected
- Verify GPIO pin number is correct
- Test LED with simple digitalWrite in loop
- Check LED polarity (anode to GPIO, cathode to GND via resistor)

### Page Loads but LED Doesn't Change

- Verify request URLs contain `/H` and `/L`
- Check Serial Monitor for received requests
- Ensure logic in sketch is correct
- Test GPIO with multimeter

### Slow Response

- Normal for simple blocking server
- Each request must complete before next
- Consider async web server for better performance

### Connection Refused

- Server may not be started (check setup())
- Port 80 may be in use
- Try different port (e.g., 8080)

### Browser Caching Issues

- Hard refresh (Ctrl+F5 or Cmd+Shift+R)
- Clear browser cache
- Try private/incognito window

## Security Considerations

⚠️ **Security Warning:**

This is a **demonstration example** with NO security:

- No authentication (anyone can control)
- No encryption (HTTP, not HTTPS)
- No input validation
- No CSRF protection
- No rate limiting

### For Production Use

1. **Add Authentication**: Username/password login
2. **Use HTTPS**: Encrypt communication
3. **Validate Input**: Check all parameters
4. **Implement CSRF Tokens**: Prevent cross-site attacks
5. **Rate Limiting**: Prevent abuse
6. **Use on Private Network**: Don't expose to internet
7. **Firewall Rules**: Restrict IP access

## Performance Notes

- **Blocking Server**: Handles one client at a time
- **Memory Usage**: ~10-20KB for basic server
- **Response Time**: Typically 50-200ms
- **Concurrent Clients**: Limited to sequential processing
- **Upgrade Path**: Consider AsyncWebServer for better performance

## Code Structure

```
setup() {
    ├─ Initialize Serial
    ├─ Configure GPIO (LED)
    ├─ Connect to WiFi
    └─ Start Server
}

loop() {
    ├─ Check for client connection
    ├─ Read HTTP request line by line
    ├─ Parse for /H or /L commands
    ├─ Control LED based on command
    ├─ Send HTTP response (HTML)
    └─ Close connection
}
```

## Alternative Implementations

### AsyncWebServer (Better Performance)

```cpp
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

server.on("/H", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(5, HIGH);
    request->send(200, "text/html", "LED ON");
});
```

### WebSocket (Real-Time)

For real-time updates without page refresh, consider WebSocket implementation.

## Notes

- Server runs on port 80 (standard HTTP)
- LED on GPIO 5 (change as needed)
- Serial baud rate: 115200
- Uses blocking socket operations
- Simple but effective for learning and prototyping
- Each page load reconnects (stateless HTTP)
- Browser may keep connection alive briefly

## Related Examples

- WiFiClient - HTTP client (accessing web pages)
- WiFiClientStaticIP - Use static IP for server
- WiFiTelnetToSerial - Telnet server example
- WiFiUDPClient - UDP communication

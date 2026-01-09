# WiFi Access Point Example

## Overview

This example demonstrates how to configure your device as a WiFi Access Point (AP). It creates a simple web server that allows clients to control an LED via HTTP requests.

## Features

- Create WiFi Access Point
- Web server on port 80
- LED control via HTTP GET requests
- Handle multiple clients
- Display connection status

## Hardware Requirements

- Tuya-supported board with WiFi capability
- Built-in LED (LED_BUILTIN)

## Configuration

Modify these values in the code:

```cpp
const char *ssid = "your_name";       // AP name
const char *password = "your_passwd";  // AP password (min 8 characters)
```

## How It Works

1. **AP Creation**: Device creates WiFi access point
2. **Server Start**: Web server starts on port 80
3. **Client Connection**: Devices can connect to the AP
4. **HTTP Server**: Serves web page with LED control links
5. **LED Control**: GET /H turns LED on, GET /L turns LED off

## Usage

1. Flash firmware to device
2. Open Serial Monitor at 115200 baud
3. Device creates AP with configured SSID
4. Connect phone/computer to the AP
5. Open browser and navigate to the displayed IP (usually 192.168.4.1)
6. Click links to control LED

## HTTP Endpoints

- `GET /H` - Turn LED on (HIGH)
- `GET /L` - Turn LED off (LOW)

## Web Interface

The server provides a simple HTML interface:
```html
Click <a href="/H">here</a> to turn ON the LED.
Click <a href="/L">here</a> to turn OFF the LED.
```

## Example Serial Output

```
Configuring access point...
AP IP address: 192.168.4.1
Server started

New Client.
GET /H HTTP/1.1
...
Client Disconnected.
```

## Network Configuration

Default AP settings:
- **IP Address**: 192.168.4.1
- **Gateway**: 192.168.4.1
- **Subnet**: 255.255.255.0
- **Port**: 80 (HTTP)

## Client Connection

From any device:
1. Search for WiFi networks
2. Connect to your AP name
3. Enter password (if set)
4. Open browser
5. Navigate to 192.168.4.1

## Customization

### Change IP Address
```cpp
IPAddress local_IP(192,168,4,22);
IPAddress gateway(192,168,4,9);
IPAddress subnet(255,255,255,0);

WiFi.softAPConfig(local_IP, gateway, subnet);
WiFi.softAP(ssid, password);
```

### Open Network (No Password)
```cpp
WiFi.softAP(ssid);  // No password parameter
```

### Set Channel and Hide SSID
```cpp
WiFi.softAP(ssid, password, channel, hidden);
// channel: 1-13
// hidden: true/false
```

## AP Capabilities

- **Max Clients**: Usually 4-8 depending on hardware
- **Range**: Typical WiFi range (50-100m open space)
- **Security**: WPA2-PSK (with password)
- **Channels**: 1-13 (region dependent)

## Security Notes

- Always use a strong password (8+ characters)
- Default password should be changed
- Monitor connected clients
- Consider implementing authentication for web interface

## Common Use Cases

- IoT device configuration
- Captive portal
- Local web interface
- Direct device control
- Temporary network sharing
- Development/testing

## Troubleshooting

### AP doesn't start
- Check SSID length (max 32 characters)
- Verify password length (min 8 characters for WPA)
- Check for conflicting WiFi operations

### Can't connect to AP
- Verify password is correct
- Check if max clients reached
- Ensure device supports the WiFi channel
- Try restarting the AP

### Web page doesn't load
- Verify IP address (check serial output)
- Ensure connected to correct AP
- Try http:// explicitly (not https://)
- Check if server started successfully

### LED doesn't respond
- Verify LED_BUILTIN is defined correctly
- Check wiring if using external LED
- Monitor serial output for GET requests

## Performance Tips

- Keep AP SSID broadcast enabled for easy discovery
- Use static IP for consistency
- Implement client timeout for inactive connections
- Add error handling for robustness

## Advanced Features

### Station + AP Mode
```cpp
WiFi.mode(WIFI_AP_STA);
```

### Get Connected Clients
```cpp
int clients = WiFi.softAPgetStationNum();
```

### AP Configuration
```cpp
wifi_config_t conf;
esp_wifi_get_config(WIFI_IF_AP, &conf);
```

## Dependencies

- WiFi library
- WiFiClient library
- WiFiAP library

## Related Examples

- WiFiClient: Connecting as a client
- CaptivePortal: Advanced AP with DNS
- SimpleWiFiServer: Basic web server

## Notes

- AP and Station modes can run simultaneously
- AP IP is usually 192.168.4.1 by default
- Web server is very basic - consider using ESPAsyncWebServer for production
- Remember to handle client disconnection gracefully

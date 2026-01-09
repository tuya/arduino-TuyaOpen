# WiFi Client Events Example

## Overview

This example demonstrates how to monitor and handle various WiFi events in real-time. It registers multiple event handlers to track WiFi connection state changes, including connection, disconnection, IP address assignment, authentication changes, and more. This is essential for building robust WiFi applications that need to respond to network state changes.

## Features

- **Comprehensive Event Monitoring**: Tracks all WiFi-related events from station mode and access point mode
- **Multiple Registration Methods**: Demonstrates different ways to register event handlers
- **Event Information Access**: Shows how to extract detailed information from events (e.g., disconnect reason, IP address)
- **Dynamic Event Management**: Supports adding and removing event handlers at runtime

## Hardware Requirements

- Tuya Open development board with WiFi capability
- USB cable for programming and serial monitoring

## Configuration

Before uploading, modify the following credentials in the sketch:

```cpp
const char* ssid     = "********";  // Your WiFi network name
const char* password = "********";  // Your WiFi password
```

## How It Works

### Event Handler Registration

The example demonstrates three different methods to register WiFi event handlers:

1. **Global Event Handler**: Handles all WiFi events
   ```cpp
   WiFi.onEvent(WiFiEvent);
   ```

2. **Specific Event Handler**: Handles only specific events
   ```cpp
   WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
   ```

3. **Lambda Function Handler**: Uses inline lambda functions
   ```cpp
   WiFiEventId_t eventID = WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info){
       // Handle event
   }, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
   ```

### Monitored Events

The example monitors various WiFi events including:

**Station Mode Events:**
- `ARDUINO_EVENT_WIFI_STA_START`: WiFi client started
- `ARDUINO_EVENT_WIFI_STA_CONNECTED`: Connected to access point
- `ARDUINO_EVENT_WIFI_STA_DISCONNECTED`: Disconnected from access point
- `ARDUINO_EVENT_WIFI_STA_GOT_IP`: Obtained IP address
- `ARDUINO_EVENT_WIFI_STA_LOST_IP`: Lost IP address
- `ARDUINO_EVENT_WIFI_SCAN_DONE`: Scan completed

**Access Point Mode Events:**
- `ARDUINO_EVENT_WIFI_AP_START`: Access point started
- `ARDUINO_EVENT_WIFI_AP_STACONNECTED`: Client connected
- `ARDUINO_EVENT_WIFI_AP_STADISCONNECTED`: Client disconnected
- `ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED`: IP assigned to client

**Other Events:**
- WPS events, IPv6 events, Ethernet events

### Event Information Extraction

The example shows how to extract detailed information from events:

```cpp
void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.println(IPAddress(info.got_ip.ip_info.ip));
}
```

For disconnection events, it extracts the reason code:
```cpp
Serial.println(info.wifi_sta_disconnected.reason);
```

## Usage

1. **Configure WiFi Credentials**: Update the `ssid` and `password` variables
2. **Upload the Sketch**: Upload to your Tuya Open board
3. **Open Serial Monitor**: Set baud rate to 115200
4. **Observe Events**: Watch as events are printed in real-time

### Expected Output

```
[WiFi-event] event:
2
WiFi client started
Wait for WiFi...

[WiFi-event] event:
4
Connected to access point

[WiFi-event] event:
7
Obtained IP address: 192.168.1.100
WiFi connected
IP address: 
192.168.1.100
```

## Event Management

### Adding Event Handlers

Event handlers can be added at any time using `WiFi.onEvent()`. The function returns an event ID that can be used to remove the handler later.

### Removing Event Handlers

To remove an event handler, use the event ID:

```cpp
WiFiEventId_t eventID = WiFi.onEvent(handler, event);
WiFi.removeEvent(eventID);
```

The example includes a commented line showing how to remove events:
```cpp
// WiFi.removeEvent(eventID);
```

## Use Cases

- **Connection Monitoring**: Track WiFi connection status for automatic reconnection
- **Network Diagnostics**: Log network events for troubleshooting
- **State Management**: Update application state based on WiFi events
- **User Notifications**: Inform users about connection status changes
- **Automatic Recovery**: Implement reconnection logic on disconnection
- **Access Point Management**: Monitor client connections in AP mode

## Troubleshooting

### No Events Displayed

- Verify WiFi credentials are correct
- Ensure the board is in range of the access point
- Check that serial monitor is open with correct baud rate (115200)

### Events Not Triggering

- Ensure event handlers are registered before calling `WiFi.begin()`
- Verify the event type matches what you're trying to monitor

### Multiple Event Fires

- Some events may trigger multiple times during connection process
- This is normal behavior - implement debouncing if needed

## Notes

- Event handlers are called from WiFi task context, not the main loop
- Keep event handler code lightweight to avoid blocking WiFi operations
- Use events for state tracking rather than polling `WiFi.status()`
- The example calls `WiFi.disconnect(true)` to clear old configurations

## Related Examples

- WiFiClient - Basic WiFi connection
- WiFiMulti - Connect to multiple access points
- WiFiScan - Scan for available networks

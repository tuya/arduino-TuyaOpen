# WiFi Client - ThingSpeak Example

## Overview

This example demonstrates how to use the WiFi client to interact with ThingSpeak IoT platform. It shows both writing data to a channel and reading data back, providing a complete example of HTTP GET requests for IoT data logging and retrieval.

## Features

- Connect to WiFi network
- Write sensor data to ThingSpeak channel
- Read data from ThingSpeak channel
- Parse JSON responses
- Handle HTTP requests and responses
- Connection timeout management

## Hardware Requirements

- Tuya IoT development board with WiFi capability
- Active internet connection
- ThingSpeak account with channel configured

## Configuration

Update the following constants in the code:

```cpp
const char* ssid = "your_wifi_ssid";        // Your WiFi SSID
const char* password = "your_wifi_password"; // Your WiFi password
const String channelID = "your_channel_id";  // Your ThingSpeak channel ID
const String writeApiKey = "your_write_key"; // Your Write API key
const String readApiKey = "your_read_key";   // Your Read API key
```

## How It Works

### 1. WiFi Connection
- Connects to specified WiFi network
- Waits for IP address assignment
- Displays connection status

### 2. Write Data
- Increments field1 value
- Sends HTTP GET request to update ThingSpeak
- Format: `/update?api_key=KEY&field1=VALUE`

### 3. Read Data
- Requests latest data from channel
- Specifies number of results and field number
- Retrieves JSON formatted data

## ThingSpeak Setup

1. Create account at [thingspeak.com](https://thingspeak.com)
2. Create a new channel
3. Configure field1 (or more fields as needed)
4. Copy Channel ID and API keys
5. Update them in the code

## Code Structure

```cpp
void setup() {
    // Initialize serial communication
    // Connect to WiFi
    // Display connection info
}

void loop() {
    // Write data to ThingSpeak
    // Read data from ThingSpeak
    // Wait before next iteration
}

void readResponse(WiFiClient *client) {
    // Handle server response
    // Implement timeout protection
    // Print received data
}
```

## Usage

1. Configure ThingSpeak credentials
2. Update WiFi credentials
3. Upload code to board
4. Open Serial Monitor (115200 baud)
5. Monitor data exchange
6. View data on ThingSpeak dashboard

## API Endpoints

**Write Data:**
```
GET /update?api_key=WRITEKEY&field1=VALUE HTTP/1.1
Host: api.thingspeak.com
```

**Read Data:**
```
GET /channels/CHANNELID/fields/FIELDNUM.json?results=NUM HTTP/1.1
Host: api.thingspeak.com
```

## Expected Output

```
Connecting to YourWiFi
...
WiFi connected
IP address: 192.168.1.100
HTTP/1.1 200 OK
...
{"channel":{"id":2005329,...
```

## Troubleshooting

**Connection Failed:**
- Verify WiFi credentials
- Check internet connectivity
- Ensure router allows HTTP traffic

**Update Failed:**
- Verify Write API key
- Check ThingSpeak rate limits (15 seconds between updates)
- Ensure field names match

**Read Failed:**
- Verify Read API key or channel public setting
- Check channel ID
- Verify field number exists

## Notes

- ThingSpeak free accounts have update rate limits
- Minimum 15 seconds between channel updates
- Maximum 3 million messages per year
- Data is publicly visible if channel is public

## Related Examples

- WiFiClientBasic - Simple TCP client
- HTTPClient - Full HTTP client library
- WiFiClientStaticIP - Using static IP configuration

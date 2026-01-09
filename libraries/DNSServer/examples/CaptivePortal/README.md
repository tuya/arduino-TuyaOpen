# Captive Portal Example

## Description

This example demonstrates how to create a captive portal using the DNSServer library. A captive portal is commonly used in WiFi networks to redirect all DNS requests to a specific web page, typically for authentication or information purposes.

The example sets up a WiFi Access Point and a DNS server that redirects all domain name queries to a simple web page displaying "Hello World!".

## Hardware Requirements

- Tuya Open development board with WiFi capability (ESP32, T2, T3, T5, etc.)

## Usage Instructions

1. Modify the WiFi credentials in the code:
   ```cpp
   WiFi.softAP("your_ssid","your_passwd");
   ```
   Replace `"your_ssid"` and `"your_passwd"` with your desired AP name and password.

2. Upload the sketch to your board.

3. Connect to the WiFi network using your mobile device or computer.

4. Try to access any website in your browser - you will be automatically redirected to the captive portal page displaying "Hello World!".

## Key Features

- **WiFi Access Point**: Creates a WiFi AP with configurable SSID and password
- **DNS Server**: Intercepts all DNS requests using wildcard domain matching (`"*"`)
- **HTTP Web Server**: Serves a simple HTML page on port 80
- **Automatic Redirection**: All DNS queries are redirected to the AP's IP address (8.8.4.4)
- **Captive Portal Detection**: Works with automatic captive portal detection on most devices

## Code Highlights

- Uses `DNSServer` library to capture all DNS requests
- Sets up custom AP IP address (8.8.4.4 for Android compatibility)
- Implements a simple HTTP server to respond with HTML content
- Processes DNS requests in the main loop for continuous operation

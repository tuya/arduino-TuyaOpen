# Basic HTTP Client Example

## Description

This example demonstrates how to make HTTP GET requests using the HTTPClient library on Tuya Open platform. It connects to a WiFi network and periodically sends HTTP GET requests to a web server, displaying the response in the Serial Monitor.

The example is useful for learning how to interact with web services and APIs over HTTP protocol.

## Hardware Requirements

- Tuya Open development board with WiFi capability (ESP32, T2, T3, T5, etc.)
- Active WiFi network connection

## Usage Instructions

1. Modify the WiFi credentials in the code:
   ```cpp
   WiFi.begin("your_ssid", "your_passwd");
   ```
   Replace `"your_ssid"` and `"your_passwd"` with your actual WiFi network credentials.

2. (Optional) Change the target URL:
   ```cpp
   http.begin("http://www.moe.gov.cn/jyb_sjzl/");
   ```

3. Upload the sketch to your board.

4. Open the Serial Monitor (115200 baud) to view the connection status and HTTP responses.

5. The board will connect to WiFi and send HTTP GET requests every 5 seconds.

## Key Features

- **WiFi Connection**: Automatically connects to specified WiFi network
- **HTTP GET Requests**: Sends GET requests with custom headers
- **Response Handling**: Receives and displays HTTP response body
- **Custom Headers**: Demonstrates adding HTTP headers (Content-Type: application/json)
- **Error Handling**: Checks for connection and request errors
- **Periodic Requests**: Repeats requests every 5 seconds

## Code Highlights

- Uses `HTTPClient` class for HTTP operations
- Checks WiFi connection status before making requests: `WiFi.status() == WSS_GOT_IP`
- Configures custom HTTP headers using `http_client_header_t` structure
- Makes GET request with: `http.GET(headers, headers_length, NULL, 0, &http_response)`
- Displays HTTP response body content via Serial
- Properly closes HTTP connection with: `http.end(&http_response)`
- Includes 4-second startup delay for system initialization

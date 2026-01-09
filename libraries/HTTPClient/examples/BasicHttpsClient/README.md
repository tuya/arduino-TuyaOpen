# Basic HTTPS Client Example

## Description

This example demonstrates how to make secure HTTPS GET requests using the HTTPClient library with SSL/TLS certificate verification. It connects to a WiFi network and sends authenticated HTTPS requests to a secure web server.

The example includes a root CA certificate for SSL/TLS verification, ensuring secure communication with the remote server. It's ideal for learning how to interact with secure web services and APIs.

## Hardware Requirements

- Tuya Open development board with WiFi capability and SSL/TLS support (ESP32, T2, T3, T5, etc.)
- Active WiFi network connection
- Sufficient flash/RAM for SSL/TLS operations

## Usage Instructions

1. Modify the WiFi credentials in the code:
   ```cpp
   WiFi.begin("your_ssid", "your_passwd");
   ```
   Replace `"your_ssid"` and `"your_passwd"` with your actual WiFi network credentials.

2. (Optional) Update the root CA certificate if connecting to a different server that uses a different certificate authority.

3. (Optional) Change the target URL:
   ```cpp
   http.begin("https://httpbin.org/get");
   ```

4. Upload the sketch to your board.

5. Open the Serial Monitor (115200 baud) to view the connection status and HTTPS responses.

6. The board will connect to WiFi and send HTTPS GET requests every 5 seconds.

## Key Features

- **Secure HTTPS Connection**: Uses SSL/TLS encryption for secure communication
- **Certificate Verification**: Includes Amazon Root CA certificate for validation
- **Custom Headers**: Adds HTTP headers (Content-Type: application/json)
- **Response Handling**: Receives and displays HTTPS response body
- **Error Handling**: Comprehensive error checking for connection and request failures
- **Debug Logging**: Uses Log library with DEBUG level for detailed output

## Code Highlights

- Includes Amazon Root CA certificate (RSA 2048 M02) for SSL/TLS verification
- Uses `Log.setLevel(LogClass::DEBUG)` for detailed logging
- Waits for WiFi connection: `WiFi.status() == WSS_GOT_IP`
- Passes CA certificate to GET request for verification:
  ```cpp
  http.GET(headers, headers_length, (uint8_t *)rootCACertificate, cacert_len, &http_response)
  ```
- Certificate length includes null terminator: `strlen(rootCACertificate) + 1`
- Displays response content from secure connection
- Properly releases resources with: `http.end(&http_response)`
- Includes 4-second startup delay and 2-second WiFi connection delay

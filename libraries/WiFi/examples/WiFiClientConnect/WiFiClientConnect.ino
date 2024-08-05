/* Wi-Fi STA Connect and Disconnect Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
   
*/
#include <WiFi.h>

const char* ssid     = "*******";
const char* password = "*******";

int btnGPIO = 8;
int btnState = false;

void setup()
{
    Serial.begin(115200);
    delay(10);

    // Set GPIO0 Boot button as input
    pinMode(btnGPIO, INPUT);

    // We start by connecting to a WiFi network
    // To debug, please enable Core Debug Level to Verbose

    Serial.println();
    Serial.print("[WiFi] Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);
// Auto reconnect is set true as default
// To set auto connect off, use the following function
//    WiFi.setAutoReconnect(false);

    // Will try for about 10 seconds (20x 500ms)
    int tryDelay = 500;
    int numberOfTries = 20;

    // Wait for the WiFi event
    while (true) {
        
        switch(WiFi.status()) {
          case WSS_NO_AP_FOUND:
            Serial.println("[WiFi] SSID not found");
            break;
          case WSS_CONN_FAIL:
            Serial.print("[WiFi] Failed - WiFi not connected! Reason: ");
            return;
            break;
          case WSS_CONN_SUCCESS:
            Serial.println("[WiFi] Scan is completed");
            break;
          case WSS_GOT_IP:
            Serial.println("[WiFi] WiFi is connected!");
            Serial.print("[WiFi] IP address: ");
            Serial.println(WiFi.localIP());
            return;
            break;
          default:
            Serial.print("[WiFi] WiFi Status: ");
            Serial.println(WiFi.status());
            break;
        }
        delay(tryDelay);
        
        if(numberOfTries <= 0){
          Serial.print("[WiFi] Failed to connect to WiFi!");
          // Use disconnect function to force stop trying to connect
          WiFi.disconnect();
          return;
        } else {
          numberOfTries--;
        }
    }
}

void loop()
{
    // Read the button state
    btnState = digitalRead(btnGPIO);
    if (btnState == LOW) {
      // Disconnect from WiFi
      Serial.println("[WiFi] Disconnecting from WiFi!");
      // This function will disconnect and turn off the WiFi (NVS WiFi data is kept)
      if(WiFi.disconnect(true, false)){
        Serial.println("[WiFi] Disconnected from WiFi!");
      }
      delay(1000);
    }

}

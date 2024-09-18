#include "tLed.h"

#include "TuyaIoT.h"
#include <Log.h>

#define ledPin  LED_BUILTIN
// Turn on LED when output low level
tLed led(ledPin, LOW);

// button
#define buttonPin         BUTTON_BUILTIN
#define buttonPressLevel  LOW
#define buttonDebounceMs  (50u)
#define buttonLongPressMs (3*1000u)

// Tuya license
#define TUYA_DEVICE_UUID    "uuidxxxxxxxxxxxxxxxx"
#define TUYA_DEVICE_AUTHKEY "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"

#define DPID_SWITCH 1

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // led
  led.off();

  // button init
  pinMode(buttonPin, INPUT_PULLUP);

  TuyaIoT.setEventCallback(tuyaIoTEventCallback);

  // license
  tuya_iot_license_t license;
  int rt = TuyaIoT.readBoardLicense(&license);
  if (OPRT_OK != rt) {
    license.uuid = TUYA_DEVICE_UUID;
    license.authkey = TUYA_DEVICE_AUTHKEY;
    Serial.println("Replace the TUYA_DEVICE_UUID and TUYA_DEVICE_AUTHKEY contents, otherwise the demo cannot work");
  }
  Serial.print("uuid: "); Serial.println(license.uuid);
  Serial.print("authkey: "); Serial.println(license.authkey);
  TuyaIoT.setLicense(license.uuid, license.authkey);

  // The "PROJECT_VERSION" comes from the "PROJECT_VERSION" field in "appConfig.json"
  TuyaIoT.begin("qhivvyqawogv04e4", PROJECT_VERSION);
}

void loop() {
  // put your main code here, to run repeatedly:
  led.update();

  // button press check
  buttonCheck();

  delay(10);
}

void tuyaIoTEventCallback(tuya_event_msg_t *event)
{
  int ledState = 0;

  tuya_event_id_t event_id = TuyaIoT.eventGetId(event);

  switch (event_id) {
    case TUYA_EVENT_BIND_START: {
      led.blink(500);
    } break;
    case TUYA_EVENT_ACTIVATE_SUCCESSED: {
      led.off();
    } break;
    case TUYA_EVENT_MQTT_CONNECTED: {
      // Update all DP
      Serial.println("---> TUYA_EVENT_MQTT_CONNECTED");
      uint8_t curState = led.getState();
      TuyaIoT.write(DPID_SWITCH, curState);
    } break;
    case TUYA_EVENT_TIMESTAMP_SYNC: {
      tal_time_set_posix(event->value.asInteger, 1);
    } break;
    case TUYA_EVENT_DP_RECEIVE_OBJ: {
      uint16_t dpNum = TuyaIoT.eventGetDpNum(event);
      for (uint16_t i = 0; i < dpNum; i++) {
        uint8_t dpid = TuyaIoT.eventGetDpId(event, i);
        switch (dpid) {
          case DPID_SWITCH: {
            TuyaIoT.read(event, DPID_SWITCH, ledState);
            Serial.print("Receive DPID_SWITCH: "); Serial.println(ledState);
            led.setState(ledState);
            TuyaIoT.write(DPID_SWITCH, ledState);
          } break;
          default : break;
        }
      }
    } break;
    default: break;
  }
}

void buttonCheck(void)
{
  static uint32_t buttonPressMs = 0;
  static uint8_t isPress = 0;

  if (digitalRead(buttonPin) == buttonPressLevel) {
    if (isPress == 0) {
      buttonPressMs = millis();
      isPress = 1;
    }

    // button debounce
    if ((1 == isPress) && ((millis() - buttonPressMs) > buttonDebounceMs)) {
      isPress = 2;
    }

    // long press check
    if ((2 == isPress) && ((millis() - buttonPressMs) >= buttonLongPressMs)) {
      isPress = 3;
      buttonLongPressStart();
    }
  } else {
    if (isPress == 2) {
      if ((millis() - buttonPressMs) < buttonLongPressMs) {
        buttonClick();
      } else {
        buttonLongPressStart();
      }
    }
    isPress = 0;
  }
}

void buttonClick()
{
  Serial.println("Button clicked");
  uint8_t ledState = led.getState();

  ledState = !ledState;
  led.setState(ledState);

  Serial.print("Upload DPID_SWITCH: "); Serial.println(ledState);
  TuyaIoT.write(DPID_SWITCH, ledState);
}

void buttonLongPressStart()
{
  Serial.println("Button long press, remove Tuya IoT device.");
  TuyaIoT.remove();
}

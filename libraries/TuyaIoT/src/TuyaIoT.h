#ifndef __TUYA_IOT_H__
#define __TUYA_IOT_H__

/******************************************************************************
 * INCLUDE
 ******************************************************************************/
#include <Arduino.h>

#include <type_traits>

extern "C" {
#include "tuya_cloud_types.h"
#include "tuya_iot.h"
}

/******************************************************************************
 * CONSTANTS
 ******************************************************************************/


/******************************************************************************
 * TYPEDEF
 ******************************************************************************/


/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/
class TuyaIoTCloudClass {
public:
  TuyaIoTCloudClass();
  ~TuyaIoTCloudClass();

  void begin(const char *pid, const char *version);
  void begin(const char *pid, const char *version, const char *uuid, const char *authKey);

  void setLicense(const char *uuid, const char *authKey);
  void setLogLevel(TAL_LOG_LEVEL_E level);

  void setEventCallback(void (*callback)(tuya_event_msg_t* event));

  // obj dp write
  int objWrite(uint8_t dpid, void* value, TIME_T timestamp = 0);

  int write(uint8_t dpid, String value, TIME_T timestamp = 0) {
    return objWrite(dpid, reinterpret_cast<void*>(const_cast<char*>(value.c_str())), timestamp);
  }

  int write(uint8_t dpid, char* value, TIME_T timestamp = 0) {
    return objWrite(dpid, reinterpret_cast<void*>(value), timestamp);
  }

  template <typename T>
  int write(uint8_t dpid, T value, TIME_T timestamp = 0) {
    return objWrite(dpid, static_cast<void *>(&value), timestamp);
  }

  // raw dp write
  int rawWrite(uint8_t dpid, uint8_t* value, uint16_t len, uint32_t timeout = 3);

  template <typename D>
  int write(D dpid, uint8_t* value, uint16_t len, uint32_t timeout = 3) {
    static_assert(std::is_integral<D>::value, "Template parameter D must be an integral type.");
    return rawWrite(static_cast<uint8_t>(dpid), value, len, timeout);
  }

  // read
  int objRead(tuya_event_msg_t* event, uint8_t dpid, dp_prop_tp_t &type, dp_value_t &value);

  template <typename T, typename D>
  int read(tuya_event_msg_t* event, D dpid, T &value) {
    static_assert(std::is_integral<T>::value, "Template parameter T must be an integral type.");
    static_assert(std::is_integral<D>::value, "Template parameter D must be an integral type.");

    int rt = OPRT_OK;

    uint8_t dpid_u8 = static_cast<uint8_t>(dpid);
    dp_prop_tp_t dp_type;
    dp_value_t dp_value;

    rt = objRead(event, dpid_u8, dp_type, dp_value);
    if (OPRT_OK != rt) {
      return rt;
    }

    switch (dp_type) {
      case (PROP_BOOL) : {
        value = static_cast<T>(dp_value.dp_bool);
      } break;
      case (PROP_VALUE) : {
        value = static_cast<T>(dp_value.dp_value);
      } break;
      case (PROP_ENUM) : {
        value = static_cast<T>(dp_value.dp_enum);
      } break;
      case (PROP_BITMAP) : {
        value = static_cast<T>(dp_value.dp_bitmap);
      } break;
      default : {
        rt = OPRT_NOT_SUPPORTED;
      } break;
    }

    return rt;
  }

  template <typename D>
  int read(tuya_event_msg_t* event, D dpid, char*& value) {
    static_assert(std::is_integral<D>::value, "Template parameter D must be an integral type.");

    int rt = OPRT_OK;
    uint8_t dpid_u8 = static_cast<uint8_t>(dpid);
    dp_prop_tp_t dp_type;
    dp_value_t dp_value;

    rt = objRead(event, dpid_u8, dp_type, dp_value);
    if (OPRT_OK != rt) {
      return rt;
    }

    if (PROP_STR != dp_type) {
      return OPRT_NOT_SUPPORTED;
    }

    value = dp_value.dp_str;

    return rt;
  }

  // read raw
  int read(tuya_event_msg_t* event, uint8_t dpid, uint8_t*& value, uint16_t& len);

  // event
  tuya_event_id_t eventGetId(tuya_event_msg_t* event);
  uint16_t eventGetDpNum(tuya_event_msg_t* event);
  uint8_t eventGetDpId(tuya_event_msg_t* event, uint16_t index);

  int remove(void);

  // license
  int readBoardLicense(tuya_iot_license_t* license);

  // device status
  bool networkCheck(void);
  bool isActivated(void);

  // time is sync
  bool isTimeSync(void);

  // UART0 is used to authorize UUID and key 
  void uartAuthInit();

  // Clear network configuration information
  void resetNetcfg();

  // check whether to reset network configuration
  int resetNetconfigCheck(void);
private:
  char _pid[MAX_LENGTH_PRODUCT_ID+1]  = {0}; // +1 is '\0'
  char _version[MAX_LENGTH_SW_VER+1]  = {0};
  char _uuid[MAX_LENGTH_UUID+1]       = {0};
  char _authKey[MAX_LENGTH_AUTHKEY+1] = {0};
};


/******************************************************************************
 * EXTERN DECLARATION
 ******************************************************************************/
extern TuyaIoTCloudClass TuyaIoT;

#endif // __TUYA_IOT_H__

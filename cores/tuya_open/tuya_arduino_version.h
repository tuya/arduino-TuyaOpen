#ifndef __TUYA_ARDUINO_VERSION_H__
#define __TUYA_ARDUINO_VERSION_H__

#ifdef __cplusplus
extern "C" {
#endif
// Major version number (X.x.x)
#define VERSION_ARDUINO_TUYA_MAJOR 1
// Minor version number (x.X.x)
#define VERSION_ARDUINO_TUYA_MINOR 1
// Patch version number (x.x.X)
#define VERSION_ARDUINO_TUYA_PATCH 6

// Version number (in numeric form)
#define VERSION_ARDUINO_TUYA ((VERSION_ARDUINO_TUYA_MAJOR << 16) | (VERSION_ARDUINO_TUYA_MINOR << 8) | VERSION_ARDUINO_TUYA_PATCH)

// Version string (in string form)
#define df2str(x) #x
#define d2str(x) df2str(x)
#define VERSION_ARDUINO_TUYA_STR d2str(VERSION_ARDUINO_TUYA_MAJOR) "." d2str(VERSION_ARDUINO_TUYA_MINOR) "." d2str(VERSION_ARDUINO_TUYA_PATCH)

#ifdef __cplusplus
}
#endif

#endif // __TUYA_ARDUINO_VERSION_H__

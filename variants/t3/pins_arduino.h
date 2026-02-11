#ifndef __PINS_ARDUINO_H__
#define __PINS_ARDUINO_H__

#include <stdint.h>

#include "tuya_cloud_types.h"

#if defined(__cplusplus) && !defined(c_plusplus)
extern "C" {
#endif // __cplusplus

// Number of pins defined in PinDescription array
#define pin0  (0u)
#define pin1  (1u)
#define pin10 (10u)
#define pin11 (11u)
#define pin12 (12u)
#define pin13 (13u)
#define pin14 (14u)
#define pin15 (15u)
#define pin16 (16u)
#define pin17 (17u)
#define pin18 (18u)
#define pin19 (19u)
#define pin24 (24u)
#define pin25 (25u)
#define pin28 (28u)
#define pin32 (32u)
#define pin34 (34u)
#define pin36 (36u)
#define pin41 (41u)
#define pin40 (40u)
#define pin48 (48u)

#define LED_BUILTIN     36
#define BUTTON_BUILTIN  32

static const uint8_t A0 = (25u);
static const uint8_t A1 = (24u);
static const uint8_t A2 = (28u);
static const uint8_t A3 = (12u);
static const uint8_t A4 = (13u);

static const uint8_t D0 = (18u);
static const uint8_t D1 = (24u);
static const uint8_t D2 = (32u);
static const uint8_t D3 = (34u);
static const uint8_t D4 = (36u);

// uart
static const uint8_t defaultSerial = 0;

// adc
#define TUYA_ADC_INVALID_VALUE (0xFFu)

TUYA_ADC_NUM_E adcPinToNum(uint8_t pin);
uint8_t adcPinToChannel(uint8_t pin);
TUYA_ADC_BASE_CFG_T adcCfgGet(uint8_t pin);

// i2c
#define SCL        (40)
#define SDA        (41)

// pwm
TUYA_PWM_NUM_E pwmPinToNum(uint8_t pin);
TUYA_PWM_BASE_CFG_T pwmCfgGet(uint8_t pin);

// SPI
#define SPI_DEFAULT_CLOCK  (8000000u)

#define PIN_SPI0_MISO       (17)
#define PIN_SPI0_MOSI       (16)
#define PIN_SPI0_SCK        (14)
#define PIN_SPI0_CS         (15)

#if defined(__cplusplus) && !defined(c_plusplus)
}
#endif // __cplusplus

#endif /* __PINS_ARDUINO_H__ */

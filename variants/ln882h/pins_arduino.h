#ifndef __PINS_ARDUINO_H__
#define __PINS_ARDUINO_H__

#include <stdint.h>

#include "tuya_cloud_types.h"

#if defined(__cplusplus) && !defined(c_plusplus)
extern "C" {
#endif // __cplusplus

// Number of pins defined in PinDescription array
#define pin0  (0u) // PB9
#define pin1  (1u) // PB8
#define pin6  (6u) // PA11
#define pin7  (7u) // PA10
#define pin8  (8u) // PA7
#define pin9  (9u) // PB5
#define pin10 (10u) // PA3
#define pin11 (11u) // PA2
#define pin14 (14u) // PA5
#define pin15 (15u) // PB7
#define pin16 (16u) // PA6
#define pin17 (17u) // PB6
#define pin20 (20u) // PA4
#define pin21 (21u) // PA9
#define pin22 (22u) // PA0
#define pin23 (23u) // PA1
#define pin24 (24u) // PB3
#define pin26 (26u) // PA12
#define pin28 (28u) // PB4

#define LED_BUILTIN     26
#define BUTTON_BUILTIN  7

static const uint8_t A0 = (23u);
static const uint8_t A1 = (22u);
static const uint8_t A2 = (24u);
static const uint8_t A3 = (26u);
static const uint8_t A4 = (28u);

static const uint8_t D0 = (6u);
static const uint8_t D1 = (7u);
static const uint8_t D2 = (8u);
static const uint8_t D3 = (24u);
static const uint8_t D4 = (26u);

// uart
static const uint8_t defaultSerial = 0;

// adc
#define TUYA_ADC_INVALID_VALUE (0xFFu)

TUYA_ADC_NUM_E adcPinToNum(uint8_t pin);
uint8_t adcPinToChannel(uint8_t pin);
TUYA_ADC_BASE_CFG_T adcCfgGet(uint8_t pin);

// pwm
TUYA_PWM_NUM_E pwmPinToNum(uint8_t pin);
TUYA_PWM_BASE_CFG_T pwmCfgGet(uint8_t pin);

// i2c
#define SCL        (20)
#define SDA        (22)

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

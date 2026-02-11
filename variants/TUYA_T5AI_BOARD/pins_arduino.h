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
#define pin2  (2u)
#define pin3  (3u)
#define pin4  (4u)
#define pin5  (5u)
#define pin6  (6u)
#define pin7  (7u)
#define pin8  (8u)
#define pin9  (9u)
#define pin12 (12u)
#define pin13 (13u)
#define pin14 (14u)
#define pin15 (15u)
#define pin16 (16u)
#define pin17 (17u)
#define pin18 (18u)
#define pin19 (19u)
#define pin20 (20u)
#define pin21 (21u)
#define pin22 (22u)
#define pin23 (23u)
#define pin24 (24u)
#define pin25 (25u)
#define pin26 (26u)
#define pin27 (27u)
#define pin28 (28u)
#define pin29 (29u)
#define pin30 (30u)
#define pin31 (31u)
#define pin32 (32u)
#define pin33 (33u)
#define pin34 (34u)
#define pin35 (35u)
#define pin36 (36u)
#define pin37 (37u)
#define pin38 (38u)
#define pin39 (39u)
#define pin40 (40u)
#define pin41 (41u)
#define pin42 (42u)
#define pin43 (43u)
#define pin44 (44u)
#define pin45 (45u)
#define pin46 (46u)
#define pin47 (47u)

#define LED_BUILTIN     1
#define BUTTON_BUILTIN  12

static const uint8_t A0 = (25u);
static const uint8_t A1 = (24u);
static const uint8_t A2 = (23u);
static const uint8_t A3 = (28u);
static const uint8_t A4 = (22u);
static const uint8_t A5 = (21u);

static const uint8_t D0 = (18u);
static const uint8_t D1 = (24u);
static const uint8_t D2 = (32u);
static const uint8_t D3 = (34u);
static const uint8_t D4 = (36u);
static const uint8_t D5 = (19u);

// uart
static const uint8_t defaultSerial = 0;
static const uint8_t TX = (11u);
static const uint8_t RX = (10u);
static const uint8_t TX1 = (0u);
static const uint8_t RX1 = (1u);
static const uint8_t TX2 = (31u);
static const uint8_t RX2 = (30u);
// static const uint8_t TX2 = (41u);
// static const uint8_t RX2 = (40u);

// adc
#define TUYA_ADC_INVALID_VALUE (0xFFu)

TUYA_ADC_NUM_E adcPinToNum(uint8_t pin);
uint8_t adcPinToChannel(uint8_t pin);
TUYA_ADC_BASE_CFG_T adcCfgGet(uint8_t pin);

// pwm
TUYA_PWM_NUM_E pwmPinToNum(uint8_t pin);
TUYA_PWM_BASE_CFG_T pwmCfgGet(uint8_t pin);

// i2c
static const uint8_t SCL = (20u);
static const uint8_t SDA = (21u);

// SPI
#define SPI_DEFAULT_CLOCK  (8000000u)

static const uint8_t SS = (15u);
static const uint8_t SCK = (14u);
static const uint8_t MOSI = (16u);
static const uint8_t MISO = (17u);

#if defined(__cplusplus) && !defined(c_plusplus)
}
#endif // __cplusplus

#endif /* __PINS_ARDUINO_H__ */

#include <Arduino.h>

#include "pins_arduino.h"

static TUYA_ADC_BASE_CFG_T sgAdcConfig[TUYA_ADC_NUM_MAX] = {
  {.ch_list.bits.ch_0=1, .ch_nums=1, .width=12, .mode=TUYA_ADC_CONTINUOUS, .type=TUYA_ADC_INNER_SAMPLE_VOL, .conv_cnt = 1}, // pin14
  {.ch_list.bits.ch_1=1, .ch_nums=1, .width=12, .mode=TUYA_ADC_CONTINUOUS, .type=TUYA_ADC_INNER_SAMPLE_VOL, .conv_cnt = 1}, // pin15
  {.ch_list.bits.ch_2=1, .ch_nums=1, .width=12, .mode=TUYA_ADC_CONTINUOUS, .type=TUYA_ADC_INNER_SAMPLE_VOL, .conv_cnt = 1}, // pin20
  {.ch_list.data=0, .ch_nums=1, .width=12, .mode=TUYA_ADC_CONTINUOUS, .type=TUYA_ADC_INNER_SAMPLE_VOL, .conv_cnt = 1}, // not support
  {.ch_list.data=0, .ch_nums=1, .width=12, .mode=TUYA_ADC_CONTINUOUS, .type=TUYA_ADC_INNER_SAMPLE_VOL, .conv_cnt = 1}, // not support
  {.ch_list.data=0, .ch_nums=1, .width=12, .mode=TUYA_ADC_CONTINUOUS, .type=TUYA_ADC_INNER_SAMPLE_VOL, .conv_cnt = 1}, // not support
  {.ch_list.data=0, .ch_nums=1, .width=12, .mode=TUYA_ADC_CONTINUOUS, .type=TUYA_ADC_INNER_SAMPLE_VOL, .conv_cnt = 1}, // not support
};

// duty 5000 = 50%
static TUYA_PWM_BASE_CFG_T sgPwmConfig[TUYA_PWM_NUM_MAX] = {
  {.polarity=TUYA_PWM_NEGATIVE, .count_mode=TUYA_PWM_CNT_UP, .duty=5000, .cycle=10000, .frequency = 1000}, // pin19
  {.polarity=TUYA_PWM_NEGATIVE, .count_mode=TUYA_PWM_CNT_UP, .duty=5000, .cycle=10000, .frequency = 1000}, // none
  {.polarity=TUYA_PWM_NEGATIVE, .count_mode=TUYA_PWM_CNT_UP, .duty=5000, .cycle=10000, .frequency = 1000}, // none
  {.polarity=TUYA_PWM_NEGATIVE, .count_mode=TUYA_PWM_CNT_UP, .duty=5000, .cycle=10000, .frequency = 1000}, // none
  {.polarity=TUYA_PWM_NEGATIVE, .count_mode=TUYA_PWM_CNT_UP, .duty=5000, .cycle=10000, .frequency = 1000}, // none
  {.polarity=TUYA_PWM_NEGATIVE, .count_mode=TUYA_PWM_CNT_UP, .duty=5000, .cycle=10000, .frequency = 1000}, // none
};

TUYA_ADC_BASE_CFG_T adcCfgGet(uint8_t pin)
{
  switch (pin) {
    case 14:
      return sgAdcConfig[0];
    case 15:
      return sgAdcConfig[1];
    case 20:
      return sgAdcConfig[2];
    default:
      return sgAdcConfig[5];
  }
}

uint8_t adcPinToChannel(uint8_t pin)
{
  switch (pin) {
    case 14:
      return 0;
    case 15:
      return 1;
    case 20:
      return 2;
    default:
      return TUYA_ADC_INVALID_VALUE;
  }
}

TUYA_ADC_NUM_E adcPinToNum(uint8_t pin)
{
  // ADC only one channel
  return TUYA_ADC_NUM_0;
}

TUYA_PWM_NUM_E pwmPinToNum(uint8_t pin)
{
  static uint8_t pwmpin_map[TUYA_PWM_NUM_MAX] = {19, 255, 255, 255, 255, 255};
  for (uint8_t i = 0; i < TUYA_PWM_NUM_MAX; i++) {
    if (pwmpin_map[i] == pin) {
      return (TUYA_PWM_NUM_E)i;
    }
    if (pwmpin_map[i] == 255) {
      pwmpin_map[i] = pin;
      return (TUYA_PWM_NUM_E)i;
    }
  }
  return TUYA_PWM_NUM_0;
}

TUYA_PWM_BASE_CFG_T pwmCfgGet(uint8_t pin)
{
  switch (pin) {
    case 19:
      return sgPwmConfig[0];
    default:
      return sgPwmConfig[5];
  }
}

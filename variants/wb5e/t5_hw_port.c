#include <Arduino.h>

#include "pins_arduino.h"

static TUYA_ADC_BASE_CFG_T sgAdcConfig[TUYA_ADC_NUM_MAX] = {
  {.ch_list.bits.ch_0=1, .ch_nums=1, .width=12, .mode=TUYA_ADC_CONTINUOUS, .type=TUYA_ADC_INNER_SAMPLE_VOL, .conv_cnt = 1}, // pin26
  {.ch_list.bits.ch_1=1, .ch_nums=1, .width=12, .mode=TUYA_ADC_CONTINUOUS, .type=TUYA_ADC_INNER_SAMPLE_VOL, .conv_cnt = 1}, // pin24
  {.ch_list.bits.ch_2=1, .ch_nums=1, .width=12, .mode=TUYA_ADC_CONTINUOUS, .type=TUYA_ADC_INNER_SAMPLE_VOL, .conv_cnt = 1}, // pin23
  {.ch_list.bits.ch_3=1, .ch_nums=1, .width=12, .mode=TUYA_ADC_CONTINUOUS, .type=TUYA_ADC_INNER_SAMPLE_VOL, .conv_cnt = 1}, // pin28
  {.ch_list.bits.ch_4=1, .ch_nums=1, .width=12, .mode=TUYA_ADC_CONTINUOUS, .type=TUYA_ADC_INNER_SAMPLE_VOL, .conv_cnt = 1}, // pin22
  {.ch_list.data=0, .ch_nums=1, .width=12, .mode=TUYA_ADC_CONTINUOUS, .type=TUYA_ADC_INNER_SAMPLE_VOL, .conv_cnt = 1}, // not support
  {.ch_list.data=0, .ch_nums=1, .width=12, .mode=TUYA_ADC_CONTINUOUS, .type=TUYA_ADC_INNER_SAMPLE_VOL, .conv_cnt = 1}, // not support
};

// duty 5000 = 50%
static TUYA_PWM_BASE_CFG_T sgPwmConfig[TUYA_PWM_NUM_MAX] = {
  {.polarity=TUYA_PWM_NEGATIVE, .count_mode=TUYA_PWM_CNT_UP, .duty=5000, .cycle=10000, .frequency = 1000}, // pin18
  {.polarity=TUYA_PWM_NEGATIVE, .count_mode=TUYA_PWM_CNT_UP, .duty=5000, .cycle=10000, .frequency = 1000}, // pin24
  {.polarity=TUYA_PWM_NEGATIVE, .count_mode=TUYA_PWM_CNT_UP, .duty=5000, .cycle=10000, .frequency = 1000}, // pin32
  {.polarity=TUYA_PWM_NEGATIVE, .count_mode=TUYA_PWM_CNT_UP, .duty=5000, .cycle=10000, .frequency = 1000}, // pin34
  {.polarity=TUYA_PWM_NEGATIVE, .count_mode=TUYA_PWM_CNT_UP, .duty=5000, .cycle=10000, .frequency = 1000}, // pin36
  {.polarity=TUYA_PWM_NEGATIVE, .count_mode=TUYA_PWM_CNT_UP, .duty=5000, .cycle=10000, .frequency = 1000} // none
};

TUYA_ADC_BASE_CFG_T adcCfgGet(uint8_t pin)
{
  switch (pin) {
    case 26:
      return sgAdcConfig[0];
    case 24:
      return sgAdcConfig[1];
    case 23:
      return sgAdcConfig[2];
    case 28:
      return sgAdcConfig[3];
    case 22:
      return sgAdcConfig[4];
    default:
      return sgAdcConfig[5];
  }
}

uint8_t adcPinToChannel(uint8_t pin)
{
  switch (pin) {
    case 25:
      return 0;
    case 24:
      return 1;
    case 23:
      return 2;
    case 28:
      return 3;
    case 22:
      return 4;
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
  switch (pin) {
    case 18:
      return TUYA_PWM_NUM_0;
    case 24:
      return TUYA_PWM_NUM_1;
    case 32:
      return TUYA_PWM_NUM_2;
    case 34:
      return TUYA_PWM_NUM_3;
    case 36:
      return TUYA_PWM_NUM_4;
    default:
      return TUYA_PWM_NUM_MAX;
  }
}

TUYA_PWM_BASE_CFG_T pwmCfgGet(uint8_t pin)
{
  switch (pin) {
    case 6:
      return sgPwmConfig[0];
    case 7:
      return sgPwmConfig[1];
    case 8:
      return sgPwmConfig[2];
    case 24:
      return sgPwmConfig[3];
    case 26:
      return sgPwmConfig[4];
    default:
      return sgPwmConfig[5];
  }
}

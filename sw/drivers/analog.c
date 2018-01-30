/*
 * analog.c
 *
 *  Created on: 28.01.2018
 *      Author: matthias
 */

#include "hal.h"
#include "config.h"

#define ADC_CHANNELS 18
#define ADC_CH_TS 16
#define ADC_CH_VREF 17
volatile adcsample_t adc_smp_raw[18];
/* filtered data are of 16bit resolution, 0-65520
 * Although written from interrupt, data is always written as a word so always valid.
 */
volatile uint16_t adc_smp_filtered[18];
uint8_t adc_filter_initialized = 0;

static void adcFilterCallback(ADCDriver *adcp, adcsample_t *buffer, size_t n);
/* ADC is clocked by 14 MHz HSI */
/* just sample all channels regularly and filter them a bit :-) */
/* conversion time ~18*18µs = ~324µs ~3 kHz */
static ADCConversionGroup allChannels = {1, 19, adcFilterCallback, NULL, ADC_CFGR1_CONT | ADC_CFGR1_RES_12BIT, 0, 3, ADC_CHSELR_CHSEL};

//Temperature sensor raw value at 30 degrees C, VDDA=3.3V
#define TEMP30_CAL_ADDR ((uint16_t*) ((uint32_t) 0x1FFFF7B8))
//Temperature sensor raw value at 110 degrees C, VDDA=3.3V
#define TEMP110_CAL_ADDR ((uint16_t*) ((uint32_t) 0x1FFFF7C2))
//Internal voltage reference raw value at 30 degrees C, VDDA=3.3V
#define VREFINT_CAL_ADDR ((uint16_t*) ((uint32_t) 0x1FFFF7BA))

#define CONDUCTION_SENSOR_1_PORT GPIOB
#define CONDUCTION_SENSOR_1_PIN GPIOB_DIN3

#define CONDUCTION_SENSOR_2_PORT GPIOB
#define CONDUCTION_SENSOR_2_PIN GPIOB_DIN2

#define CONDUCTION_SENSOR_COMMON_PORT GPIOB
#define CONDUCTION_SENSOR_COMMON_PIN GPIOB_DIN4
#define CONDUCTION_SENSOR_COMMON_ANALOG_CH 9

#define ADC_MAX (16*4095)

/* Filters raw data with f(o, n) = 1/16 o + 15/16 n; also increases resolution to 16 bits */
static void adcFilterCallback(ADCDriver *adcp, adcsample_t *buffer, size_t n)
{
  if(!adc_filter_initialized)
  {
    for(int i = 0; i < ADC_CHANNELS; ++i)
    {
      adc_smp_filtered[i] = adc_smp_raw[i] * 16;
    }
    adc_filter_initialized = 1;
  } else
  {
    for(int i = 0; i < ADC_CHANNELS; ++i)
    {
      uint32_t smp = adc_smp_filtered[i] / 16;
      adc_smp_filtered[i] = buffer[i] + 15*smp;
    }
  }
}

void analog_init(void)
{
  adcStart(&ADCD1, NULL);
  adcSTM32EnableTS();
  adcSTM32EnableVREF();
  adcStartConversion(&ADCD1, &allChannels, adc_smp_raw, 1);
  if(config_get_uint(CONFIG_HAS_ANALOG_CONDUCTION_SENSOR)) {
    /* Conduction sensor at DIN4 (AIN) vs DIN3/DIN2.
     * should be driven with alternating current to avoid electrolysis.
     * Init: All lines to same level to avoid any current flow
     */
    palSetPadMode(CONDUCTION_SENSOR_1_PORT, CONDUCTION_SENSOR_1_PIN, PAL_MODE_INPUT_PULLUP);
    palSetPadMode(CONDUCTION_SENSOR_2_PORT, CONDUCTION_SENSOR_2_PIN, PAL_MODE_INPUT_PULLUP);
    palSetPadMode(CONDUCTION_SENSOR_COMMON_PORT, CONDUCTION_SENSOR_COMMON_PIN, PAL_MODE_INPUT_PULLUP);
  }
}

int analog_meassure_conduction(void)
{
  /* polarity defines low(0) or high(1) on common port side */
  static int polarity = 1;
  iomode_t passive_mode = PAL_MODE_INPUT_ANALOG;
  iomode_t input_mode = polarity ? PAL_MODE_INPUT_PULLUP : PAL_MODE_INPUT_PULLDOWN;

  uint16_t sensor1, sensor2;

  /* first measure sensor 1 */
  palSetPadMode(CONDUCTION_SENSOR_COMMON_PORT, CONDUCTION_SENSOR_COMMON_PIN, input_mode);
  palSetPadMode(CONDUCTION_SENSOR_2_PORT, CONDUCTION_SENSOR_2_PIN, passive_mode);
  palSetPadMode(CONDUCTION_SENSOR_1_PORT, CONDUCTION_SENSOR_1_PIN, PAL_MODE_OUTPUT_PUSHPULL);
  if(polarity)
  {
    palClearPad(CONDUCTION_SENSOR_1_PORT, CONDUCTION_SENSOR_1_PIN);
  } else
  {
    palSetPad(CONDUCTION_SENSOR_1_PORT, CONDUCTION_SENSOR_1_PIN);
  }
  chThdSleep(MS2ST(1));
  sensor1 = adc_smp_raw[CONDUCTION_SENSOR_COMMON_ANALOG_CH];

  /* then measure sensor 2 */
  palSetPadMode(CONDUCTION_SENSOR_1_PORT, CONDUCTION_SENSOR_1_PIN, passive_mode);
  palSetPadMode(CONDUCTION_SENSOR_2_PORT, CONDUCTION_SENSOR_2_PIN, PAL_MODE_OUTPUT_PUSHPULL);
  if(polarity)
  {
    palClearPad(CONDUCTION_SENSOR_2_PORT, CONDUCTION_SENSOR_2_PIN);
  } else
  {
    palSetPad(CONDUCTION_SENSOR_2_PORT, CONDUCTION_SENSOR_2_PIN);
  }
  chThdSleep(MS2ST(1));
  sensor2 = adc_smp_raw[CONDUCTION_SENSOR_COMMON_ANALOG_CH];

  /* finally, passivate & change polarity */
  palSetPadMode(CONDUCTION_SENSOR_1_PORT, CONDUCTION_SENSOR_1_PIN, input_mode);
  palSetPadMode(CONDUCTION_SENSOR_2_PORT, CONDUCTION_SENSOR_2_PIN, input_mode);
  polarity = !polarity;

  return ((uint32_t)sensor1 << 16) | sensor2;
}

int analog_conduction_closed(uint8_t sensor) {
  return 0;
}

/* read internal temperature sensor in centi-degrees */
int analog_get_internal_ts(void)
{
  int32_t t = ((adc_smp_filtered[ADC_CH_TS]/16 * (*VREFINT_CAL_ADDR)) / (adc_smp_filtered[ADC_CH_VREF]/16)) - (int32_t)*TEMP30_CAL_ADDR;
  t *= 11000 - 3000;
  t = t / (int32_t)(*TEMP110_CAL_ADDR - *TEMP30_CAL_ADDR);
  t += 3000;
  return t;
}

int analog_get_vdda(void)
{
  return (16 * 3300 * (*VREFINT_CAL_ADDR)) / adc_smp_filtered[ADC_CH_VREF];
}

/*
 * analog.c
 *
 *  Created on: 28.01.2018
 *      Author: matthias
 */

#include "hal.h"
#include "config.h"
#include "analog.h"
#include "util.h"

#define ADC_CH_TS 16
#define ADC_CH_VREF 17
volatile adcsample_t adc_smp_raw[ANALOG_CHANNELS];
/* filtered data are of 16bit resolution, 0-65520
 * Although written from interrupt, data is always written as a word so always valid.
 */
volatile uint16_t adc_smp_filtered[ANALOG_CHANNELS];
volatile uint16_t analog_input_debug;
uint8_t adc_filter_initialized = 0;

static void adcFilterCallback(ADCDriver *adcp, adcsample_t *buffer, size_t n);
/* ADC is clocked by 14 MHz HSI */
/* just sample all channels regularly and filter them a bit :-) */
/* conversion time ~18*18µs = ~324µs ~3 kHz */
static ADCConversionGroup allChannels = {1, 18, adcFilterCallback, NULL, ADC_CFGR1_CONT | ADC_CFGR1_RES_12BIT, 0, 3, ADC_CHSELR_CHSEL & ~ADC_CHSELR_CHSEL18};

//Temperature sensor raw value at 30 degrees C, VDDA=3.3V
#define TEMP30_CAL_ADDR ((uint16_t*) ((uint32_t) 0x1FFFF7B8))
//Temperature sensor raw value at 110 degrees C, VDDA=3.3V
#define TEMP110_CAL_ADDR ((uint16_t*) ((uint32_t) 0x1FFFF7C2))
//Internal voltage reference raw value at 30 degrees C, VDDA=3.3V
#define VREFINT_CAL_ADDR ((uint16_t*) ((uint32_t) 0x1FFFF7BA))

void analog_debug_input(uint16_t channelMask)
{
  analog_input_debug =  channelMask;
}

static void analog_filter_resetS(uint8_t channel)
{
  adc_smp_filtered[channel] = adc_smp_raw[channel] * 16;
}

void analog_filter_reset(uint16_t channels)
{
  chSysLock();
  for(uint8_t i = 0; i <= 15; ++i)
  {
    if(channels & (1 << i))
    {
      analog_filter_resetS(i);
    }
  }
  chSysUnlock();
}


/* Filters raw data with f(o, n) = 1/16 o + 15/16 n; also increases resolution to 16 bits */
static void adcFilterCallback(ADCDriver *adcp, adcsample_t *buffer, size_t n)
{
  if(!adc_filter_initialized)
  {
    for(int i = 0; i < ANALOG_CHANNELS; ++i)
    {
      analog_filter_resetS(i);
    }
    adc_filter_initialized = 1;
  } else
  {
    for(int i = 0; i < ANALOG_CHANNELS; ++i)
    {
      uint32_t smp = (adc_smp_filtered[i] + 8) / 16;
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

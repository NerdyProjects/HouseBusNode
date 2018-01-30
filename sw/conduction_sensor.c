/*
 * conduction_sensor.c
 *
 *  Created on: 30.01.2018
 *      Author: matthias
 */
#include <stdint.h>
#include "drivers/analog.h"
#include "config.h"
#include "qfplib.h"

#define CONDUCTION_SENSOR_1_PORT GPIOB
#define CONDUCTION_SENSOR_1_PIN GPIOB_DIN3

#define CONDUCTION_SENSOR_2_PORT GPIOB
#define CONDUCTION_SENSOR_2_PIN GPIOB_DIN2

#define CONDUCTION_SENSOR_COMMON_PORT GPIOB
#define CONDUCTION_SENSOR_COMMON_PIN GPIOB_DIN4
#define CONDUCTION_SENSOR_COMMON_ANALOG_CH 9

uint16_t SensorRaw[2];
uint16_t SensorRef[2];

void conduction_init(void)
{
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

void conduction_acquire(void)
{
  /* polarity defines low(0) or high(1) on common port side */
  static int polarity = 1;
  iomode_t passive_mode = PAL_MODE_INPUT_ANALOG;
  iomode_t input_mode = polarity ? PAL_MODE_INPUT_PULLUP : PAL_MODE_INPUT_PULLDOWN;

  palSetPadMode(CONDUCTION_SENSOR_COMMON_PORT, CONDUCTION_SENSOR_COMMON_PIN, input_mode);

  /* first measure reference (open connection) */
  palSetPadMode(CONDUCTION_SENSOR_2_PORT, CONDUCTION_SENSOR_2_PIN, passive_mode);
  palSetPadMode(CONDUCTION_SENSOR_1_PORT, CONDUCTION_SENSOR_1_PIN, passive_mode);
  chThdSleep(MS2ST(1));
  analog_filter_reset(CONDUCTION_SENSOR_COMMON_ANALOG_CH);
  chThdSleep(MS2ST(5));
  SensorRef[0] = adc_smp_filtered[CONDUCTION_SENSOR_COMMON_ANALOG_CH];

  /* first measure sensor 1 */
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
  analog_filter_reset(CONDUCTION_SENSOR_COMMON_ANALOG_CH);
  chThdSleep(MS2ST(5));
  SensorRaw[0] = adc_smp_filtered[CONDUCTION_SENSOR_COMMON_ANALOG_CH];

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
  analog_filter_reset(CONDUCTION_SENSOR_COMMON_ANALOG_CH);
  chThdSleep(MS2ST(5));
  SensorRaw[1] = adc_smp_filtered[CONDUCTION_SENSOR_COMMON_ANALOG_CH];

  /* measure reference again to detect errors */
  palSetPadMode(CONDUCTION_SENSOR_2_PORT, CONDUCTION_SENSOR_2_PIN, passive_mode);
  palSetPadMode(CONDUCTION_SENSOR_1_PORT, CONDUCTION_SENSOR_1_PIN, passive_mode);
  chThdSleep(MS2ST(1));
  analog_filter_reset(CONDUCTION_SENSOR_COMMON_ANALOG_CH);
  chThdSleep(MS2ST(5));
  SensorRef[1] = adc_smp_filtered[CONDUCTION_SENSOR_COMMON_ANALOG_CH];

  /* finally, passivate & change polarity */
  palSetPadMode(CONDUCTION_SENSOR_COMMON_PORT, CONDUCTION_SENSOR_COMMON_PIN, passive_mode);
  palSetPadMode(CONDUCTION_SENSOR_1_PORT, CONDUCTION_SENSOR_1_PIN, passive_mode);
  palSetPadMode(CONDUCTION_SENSOR_2_PORT, CONDUCTION_SENSOR_2_PIN, passive_mode);
  polarity = !polarity;
}

/*
 * MAX 120 REF 100 SIG 10 -> SIG/REF = 0.1
 * MAX 120 REF 10 SIG 100 -> 1/SIG/REF
 */
uint8_t conduction_evaluate(uint8_t sensor, uint8_t *quality)
{
  if(((SensorRef[0] < (ANALOG_MAX/4)) && (SensorRef[1] < (ANALOG_MAX/4)))
      || ((SensorRef[0] > (ANALOG_MAX/4*3)) && (SensorRef[1] > (ANALOG_MAX/4*3))))
  {
    /* reference not in same quadrants -> error */
    if(quality)
    {
      *quality = 0;
    }
    return 0xFF;
  }
  float v = qfp_fdiv(qfp_int2float(SensorRaw[sensor]), ANALOG_MAX);

  return qfp_fcmp(v, 0.3f);
}

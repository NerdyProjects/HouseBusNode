/*
 * conduction_sensor.c
 *
 *  Created on: 30.01.2018
 *      Author: matthias
 */
#include <stdint.h>
#include "util.h"
#include "drivers/analog.h"
#include "config.h"
#include "qfplib.h"
#include "conduction_sensor.h"

#define CONDUCTION_SENSOR_1_PORT GPIOB
#define CONDUCTION_SENSOR_1_PIN GPIOB_DIN3

#define CONDUCTION_SENSOR_2_PORT GPIOB
#define CONDUCTION_SENSOR_2_PIN GPIOB_DIN2

#define CONDUCTION_SENSOR_COMMON_PORT GPIOB
#define CONDUCTION_SENSOR_COMMON_PIN GPIOB_DIN4
#define CONDUCTION_SENSOR_COMMON_ANALOG_CH 9

#define CONDUCTION_THRESHOLD 200

#define CONDUCTION_MAX_SENSORS 2

static uint16_t SensorRaw[CONDUCTION_MAX_SENSORS];
static uint8_t SensorRefPolarity;
static uint8_t error;
static uint8_t conduction_present_num;
MUTEX_DECL(conductionSensorDataMtx);

void conduction_init(void)
{
  conduction_present_num = config_get_uint(CONFIG_HAS_ANALOG_CONDUCTION_SENSOR);
  if(conduction_present_num) {
    /* Conduction sensor at DIN4 (AIN) vs DIN3/DIN2.
     * should be driven with alternating current to avoid electrolysis.
     * Init: All lines to same level to avoid any current flow
     */
    palSetPadMode(CONDUCTION_SENSOR_1_PORT, CONDUCTION_SENSOR_1_PIN, PAL_MODE_INPUT_PULLUP);
    palSetPadMode(CONDUCTION_SENSOR_2_PORT, CONDUCTION_SENSOR_2_PIN, PAL_MODE_INPUT_PULLUP);
    palSetPadMode(CONDUCTION_SENSOR_COMMON_PORT, CONDUCTION_SENSOR_COMMON_PIN, PAL_MODE_INPUT_PULLUP);
    if(conduction_present_num > CONDUCTION_MAX_SENSORS)
    {
      conduction_present_num = CONDUCTION_MAX_SENSORS;
    }
  }
  /* initialize data to realworld situation :-) */
  conduction_acquire();
}

/* checks v to be an acceptable reference for polarity.
 * Sets a global error state if failed.
 */
static void checkReferenceLevel(uint16_t v, uint8_t polarity)
{
  if((polarity && v < (ANALOG_MAX / 16) * 11)
      || (!polarity && v > (ANALOG_MAX / 16) * 5))
  {
    error = 1;
  }
}

void conduction_acquire(void)
{
  /* polarity defines low(0) or high(1) on common port side */
  static uint8_t polarity = 1;
  uint16_t ref;
  iomode_t passive_mode = PAL_MODE_INPUT_ANALOG;
  iomode_t input_mode = polarity ? PAL_MODE_INPUT_PULLUP : PAL_MODE_INPUT_PULLDOWN;

  chMtxLock(&conductionSensorDataMtx);
  palSetPadMode(CONDUCTION_SENSOR_COMMON_PORT, CONDUCTION_SENSOR_COMMON_PIN, input_mode);

  /* first measure reference (open connection) */
  palSetPadMode(CONDUCTION_SENSOR_2_PORT, CONDUCTION_SENSOR_2_PIN, passive_mode);
  palSetPadMode(CONDUCTION_SENSOR_1_PORT, CONDUCTION_SENSOR_1_PIN, passive_mode);
  chThdSleep(TIME_MS2I(1));
  analog_filter_reset(1 << CONDUCTION_SENSOR_COMMON_ANALOG_CH);
  chThdSleep(TIME_MS2I(5));
  ref = adc_smp_filtered[CONDUCTION_SENSOR_COMMON_ANALOG_CH];
  checkReferenceLevel(ref, polarity);

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
  chThdSleep(TIME_MS2I(1));
  analog_filter_reset(1 << CONDUCTION_SENSOR_COMMON_ANALOG_CH);
  chThdSleep(TIME_MS2I(5));
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
  chThdSleep(TIME_MS2I(1));
  analog_filter_reset(1 << CONDUCTION_SENSOR_COMMON_ANALOG_CH);
  chThdSleep(TIME_MS2I(5));
  SensorRaw[1] = adc_smp_filtered[CONDUCTION_SENSOR_COMMON_ANALOG_CH];

  /* measure reference again to detect errors */
  palSetPadMode(CONDUCTION_SENSOR_2_PORT, CONDUCTION_SENSOR_2_PIN, passive_mode);
  palSetPadMode(CONDUCTION_SENSOR_1_PORT, CONDUCTION_SENSOR_1_PIN, passive_mode);
  chThdSleep(TIME_MS2I(1));
  analog_filter_reset(1 << CONDUCTION_SENSOR_COMMON_ANALOG_CH);
  chThdSleep(TIME_MS2I(5));
  ref = adc_smp_filtered[CONDUCTION_SENSOR_COMMON_ANALOG_CH];
  checkReferenceLevel(ref, polarity);

  SensorRefPolarity = polarity;
  /* finally, passivate & change polarity */
  palSetPadMode(CONDUCTION_SENSOR_COMMON_PORT, CONDUCTION_SENSOR_COMMON_PIN, passive_mode);
  palSetPadMode(CONDUCTION_SENSOR_1_PORT, CONDUCTION_SENSOR_1_PIN, passive_mode);
  palSetPadMode(CONDUCTION_SENSOR_2_PORT, CONDUCTION_SENSOR_2_PIN, passive_mode);
  polarity = !polarity;
  chMtxUnlock(&conductionSensorDataMtx);
}

uint8_t conduction_getClearError(void)
{
  uint8_t e = error;
  error = 0;
  return e;
}


bool conduction_evaluate(uint8_t sensor, uint8_t *quality)
{
  chMtxLock(&conductionSensorDataMtx);
  float raw = qfp_int2float(SensorRaw[sensor]);
  float v = qfp_fdiv(raw, ANALOG_MAX);
  if(SensorRefPolarity) {
    v = qfp_fsub(1.0f, v);
  }
  chMtxUnlock(&conductionSensorDataMtx);

  int q = qfp_float2int(qfp_fmul(255.0f, v));
  if(quality)
  {
    *quality = q;
  }

  return q > CONDUCTION_THRESHOLD;
}

uint8_t conduction_num_sensors(void)
{
  return conduction_present_num;
}

//static void broadcast_conduction_data(uint8_t error, uint8_t state, uint8_t num, uint8_t *quality)
//{
//  uint8_t buffer[HOMEAUTOMATION_CONDUCTION_SENSOR_MESSAGE_SIZE];
//  static uint8_t transfer_id = 0;
//  /* bit0: Error in data */
//  buffer[0] = state; /* Bit 0-6: state */
//  if(error)
//  {
//    buffer[0] |= 0x80; /* Bit 7: error */
//  }
//  for(int i = 0; i < num; ++i) {
//    buffer[1+i] = quality[i];
//  }
//  canardLockBroadcast(&canard,
//          HOMEAUTOMATION_CONDUCTION_SENSOR_DATA_TYPE_SIGNATURE,
//          HOMEAUTOMATION_CONDUCTION_SENSOR_DATA_TYPE_ID, &transfer_id,
//          CANARD_TRANSFER_PRIORITY_LOW, buffer, 1 + num);
//}

///* Conduction sensor */
//    uint8_t num = conduction_num_sensors();
//    if(num)
//    {
//      uint8_t state = 0;
//      uint8_t q[8];
//      conduction_acquire();
//      uint8_t error = conduction_getClearError();
//      for(int i = 0; i < num; ++i)
//      {
//        if(conduction_evaluate(i, &q[i]))
//        {
//          state |= (1 << i);
//        }
//      }
//      broadcast_conduction_data(error, state, num, q);
//    }

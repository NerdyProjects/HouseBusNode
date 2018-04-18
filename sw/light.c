/*
 * light.c
 *
 *  Created on: 13.04.2018
 *      Author: matthias
 */


#include "config.h"
#include "drivers/pwm.h"
#include "time_data.h"

/* Hallway light beginning from door .. backyard */
#define HALLWAY_START 1
#define HALLWAY_END 5

#define STAIRCASE_K20_1 6
#define STAIRCASE_K20_2 7

/* GPIOB 4 is motion sensor input, active high */
#define HALLWAY_MOTION_SENSOR_PORT GPIOB
#define HALLWAY_MOTION_SENSOR_PAD 4
#define HALLWAY_MOTION_SENSOR_MAX_SECONDS 120

static volatile uint32_t hallway_motion_sensor_on_seconds = 9999;

static void motion_sensor_callback(void* arg)
{
  hallway_motion_sensor_on_seconds = 0;
  for(int i = HALLWAY_START; i <= HALLWAY_END; ++i)
  {
    pwm_set_dc(i, 1500);
  }

  if (palReadPad(HALLWAY_MOTION_SENSOR_PORT, HALLWAY_MOTION_SENSOR_PAD) == PAL_HIGH)
  {
    hallway_motion_sensor_on_seconds = 0;
    for(int i = HALLWAY_START; i <= HALLWAY_END; ++i)
    {
      pwm_set_dc(i, 1500);
    }
  }
}

void light_init(void)
{
  if(!config_get_uint(CONFIG_HAS_LIGHT_CONTROL)) return;

  pwm_init();
  for(int i = HALLWAY_START; i <= HALLWAY_END; ++i)
  {
    pwm_set_dc(i, 150);
  }
  pwm_set_dc(STAIRCASE_K20_1, 1500);
  pwm_set_dc(STAIRCASE_K20_2, 40);

  palSetPadMode(HALLWAY_MOTION_SENSOR_PORT, HALLWAY_MOTION_SENSOR_PAD, PAL_MODE_INPUT);

  chSysLock();
  palSetPadCallbackI(HALLWAY_MOTION_SENSOR_PORT, HALLWAY_MOTION_SENSOR_PAD, motion_sensor_callback, NULL);
  chSysUnlock();
}

void light_tick(void)
{
  if(!config_get_uint(CONFIG_HAS_LIGHT_CONTROL)) return;

  bool hallway_motion_sensor_on = hallway_motion_sensor_on_seconds < HALLWAY_MOTION_SENSOR_MAX_SECONDS;

  if(hallway_motion_sensor_on)
  {
    for(int i = HALLWAY_START; i <= HALLWAY_END; ++i)
    {
      pwm_set_dc(i, 1500);
    }
    hallway_motion_sensor_on_seconds++;
    return;
  }

  uint64_t hour = time_data_hour(); // UTC

  if(hour >= 17 || hour <= 3)
  {
    if(hour <= 22)
    {
      // evening mode
      for(int i = HALLWAY_START; i <= HALLWAY_END; ++i)
      {
        pwm_set_dc(i, 300);
      }
      pwm_set_dc(STAIRCASE_K20_1, 1500);
      pwm_set_dc(STAIRCASE_K20_2, 100);
    }
    else
    {
      // night mode
      for(int i = HALLWAY_START; i <= HALLWAY_END; ++i)
      {
        pwm_set_dc(i, 100);
      }
      pwm_set_dc(STAIRCASE_K20_1, 100);
      pwm_set_dc(STAIRCASE_K20_2, 40);
    }
  }
  else {
    // day mode
    for(int i = HALLWAY_START; i <= HALLWAY_END; ++i)
    {
      pwm_set_dc(i, 0);
    }
    pwm_set_dc(STAIRCASE_K20_1, 0);
    pwm_set_dc(STAIRCASE_K20_2, 0);
  }
}

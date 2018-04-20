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
#define HALLWAY_MOTION_SENSOR_MAX_SECONDS 7

static volatile uint32_t hallway_motion_sensor_trigger = 0;
static uint32_t hallway_target_light = 0;

static void motion_sensor_callback(void* arg)
{
  if (palReadPad(HALLWAY_MOTION_SENSOR_PORT, HALLWAY_MOTION_SENSOR_PAD) == PAL_HIGH)
  {
    hallway_motion_sensor_trigger = 1;
  }
}

void light_init(void)
{
  if(!config_get_uint(CONFIG_HAS_LIGHT_CONTROL)) return;

  pwm_init();
  for(int i = HALLWAY_START; i <= HALLWAY_END; ++i)
  {
    pwm_set_dc(i, 0.1*65000);
  }
  pwm_set_dc(STAIRCASE_K20_1, 0.1*65000);
  pwm_set_dc(STAIRCASE_K20_2, 0.1*65000);

  palSetPadMode(HALLWAY_MOTION_SENSOR_PORT, HALLWAY_MOTION_SENSOR_PAD, PAL_MODE_INPUT_PULLUP);
  pal_lld_enablepadevent(HALLWAY_MOTION_SENSOR_PORT, HALLWAY_MOTION_SENSOR_PAD, PAL_EVENT_MODE_RISING_EDGE);

  chSysLock();
  palSetPadCallbackI(HALLWAY_MOTION_SENSOR_PORT, HALLWAY_MOTION_SENSOR_PAD, motion_sensor_callback, NULL);
  chSysUnlock();
}

int get_fade_val(int step, int fade_from, int fade_to, int length)
{
  if(step < 0)
  {
    return fade_from;
  }
  if(step > length)
  {
    return fade_to;
  }
  int diff = fade_to - fade_from;
  return fade_from + ((diff * step) / length);
}

void light_fast_tick(void)
{
  static int hallway_animation;
  static int hallway_animation_counter;
  static int hallway_animation_tmp;
  static int hallway_animation_target_light;
  static int hallway_light;
  static systime_t hallway_motion_sensor_on_time;
  if(hallway_motion_sensor_trigger)
  {
    hallway_motion_sensor_trigger = 0;
    hallway_motion_sensor_on_time = chVTGetSystemTime();
    if(hallway_animation != 1)
    {
      hallway_animation = 1;
      hallway_animation_counter = 0;
      hallway_animation_tmp = hallway_light;
      hallway_animation_target_light = 20000;
    }
  }
  if(hallway_animation == 1)
  {
    hallway_light = get_fade_val(hallway_animation_counter, hallway_animation_tmp, hallway_animation_target_light, 150);
    for(int i = 0; i <= (HALLWAY_END - HALLWAY_START); ++i)
    {
      pwm_set_dc(HALLWAY_START + i, get_fade_val(hallway_animation_counter - 30 * i, hallway_animation_tmp, hallway_animation_target_light, 150));
    }
    hallway_animation_counter++;
    if(hallway_animation_counter > (150 + (HALLWAY_END-HALLWAY_START) * 30))
    {
      hallway_animation = 0;
    }
  } else
  {
    if(hallway_motion_sensor_on_time + TIME_S2I(HALLWAY_MOTION_SENSOR_MAX_SECONDS) < chVTGetSystemTime())
    {
      hallway_light = hallway_target_light;
    } else
    {
      hallway_light = hallway_animation_target_light;
    }
    for(int i = 0; i <= (HALLWAY_END - HALLWAY_START); ++i)
    {
      pwm_set_dc(HALLWAY_START + i, hallway_light);
    }
  }
}

void light_tick(void)
{
  if(!config_get_uint(CONFIG_HAS_LIGHT_CONTROL)) return;

  uint64_t hour = time_data_hour(); // UTC

  if(hour >= 18 || hour <= 3)
  {
    if(hour <= 21)
    {
      // evening mode
      hallway_target_light = 2000;
      pwm_set_dc(STAIRCASE_K20_1, 6000);
      pwm_set_dc(STAIRCASE_K20_2, 1500);
    }
    else
    {
      // night mode
      hallway_target_light = 70;
      pwm_set_dc(STAIRCASE_K20_1, 350);
      pwm_set_dc(STAIRCASE_K20_2, 70);
    }
  }
  else {
    // day mode
    hallway_target_light = 10;
    pwm_set_dc(STAIRCASE_K20_1, 10);
    pwm_set_dc(STAIRCASE_K20_2, 10);
  }
}

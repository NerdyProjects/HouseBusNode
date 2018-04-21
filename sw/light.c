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

#define ANIMATION_NONE 0
#define ANIMATION_FADE 1

static volatile uint32_t hallway_motion_sensor_trigger = 0;

typedef struct {
  int animation;
  int from;
  int to;
  int length;
  int counter;
} animation_state_t;

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

  palSetPadMode(HALLWAY_MOTION_SENSOR_PORT, HALLWAY_MOTION_SENSOR_PAD, PAL_MODE_INPUT);

  chSysLock();
  palEnablePadEventI(HALLWAY_MOTION_SENSOR_PORT, HALLWAY_MOTION_SENSOR_PAD, PAL_EVENT_MODE_RISING_EDGE);
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

int animate(uint16_t *brightness, uint8_t brightness_length, animation_state_t *state)
{
  switch (state->animation)
  {
    case(ANIMATION_FADE):
      if (state->counter + (brightness_length - 1) * 30 >= state->length) return 0;

      for(int i = 0; i < brightness_length; ++i)
      {
        brightness[i] = get_fade_val(state->counter - 30 * i, state->from, state->to, state->length);
      }
      state->counter++;
      break;
  }

  return 1;
}

void start_animation_fade(animation_state_t *state, int from, int to, int length)
{
  state->animation = ANIMATION_FADE;
  state->counter = 0;
  state->from = from;
  state->to = to;
  state->length = length;
}

void light_fast_tick(void)
{
  static int hallway_target_light = 0;
  static int next_hallway_target_light = 0;
  static int hallway_motion_sensor_target_light = 20000;
  static int hallway_animation = ANIMATION_NONE;
  static animation_state_t animation;

  static systime_t hallway_motion_sensor_on_time;

  if(!config_get_uint(CONFIG_HAS_LIGHT_CONTROL)) return;

  uint8_t hour = time_hour; // UTC

  if(hour >= 18 || hour <= 3)
  {
    if(hour <= 21)
    {
      // evening mode
      next_hallway_target_light = 2000;
      pwm_set_dc(STAIRCASE_K20_1, 6000);
      pwm_set_dc(STAIRCASE_K20_2, 1500);
    }
    else
    {
      // night mode
      next_hallway_target_light = 70;
      pwm_set_dc(STAIRCASE_K20_1, 350);
      pwm_set_dc(STAIRCASE_K20_2, 70);
    }
  }
  else {
    // day mode
    next_hallway_target_light = 10;
    pwm_set_dc(STAIRCASE_K20_1, 10);
    pwm_set_dc(STAIRCASE_K20_2, 10);
  }

  // check if motion sensor was triggered
  if(hallway_motion_sensor_trigger)
  {
    hallway_motion_sensor_trigger = 0;
    hallway_motion_sensor_on_time = chVTGetSystemTime();
  }

  // override target brightness if motion sensor is on
  if (hallway_motion_sensor_on_time + TIME_S2I(HALLWAY_MOTION_SENSOR_MAX_SECONDS) < chVTGetSystemTime())
  {
    next_hallway_target_light = hallway_motion_sensor_target_light;
  }

  // accept next_hallway_target_light, start animation if changed
  if (next_hallway_target_light != hallway_target_light)
  {
    if(hallway_animation == ANIMATION_NONE)
    {
      start_animation_fade(&animation, hallway_target_light, next_hallway_target_light, 150);
      hallway_animation = ANIMATION_FADE;
    }
    hallway_target_light = next_hallway_target_light;
  }

  if(hallway_animation != ANIMATION_NONE)
  {
    // animation in progress
    uint8_t brightness_length = (HALLWAY_END - HALLWAY_START) + 1;
    uint16_t brightness[brightness_length - 1];

    if (animate(brightness, brightness_length, &animation))
    {
      for(int i = 0; i < brightness_length; ++i)
      {
        pwm_set_dc(HALLWAY_START + i, brightness[i]);
      }
    }
    else {
      // animation ends
      hallway_animation = ANIMATION_NONE;
    }
  }
  else
  {
    // constant brightness
    for(int i = 0; i <= (HALLWAY_END - HALLWAY_START); ++i)
    {
      pwm_set_dc(HALLWAY_START + i, hallway_target_light);
    }
  }
}

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
#define HALLWAY_LIGHTS (HALLWAY_END - HALLWAY_START + 1)

#define STAIRCASE_K20_1 6
#define STAIRCASE_K20_2 7

/* GPIOB 4 is motion sensor input, active high */
#define HALLWAY_MOTION_SENSOR_PORT GPIOB
#define HALLWAY_MOTION_SENSOR_PAD 4
#define HALLWAY_MOTION_SENSOR_MAX_SECONDS 30

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
      if (state->counter >= state->length) return 0;

      for(int i = 0; i < brightness_length; ++i)
      {
        brightness[i] = get_fade_val(state->counter - 30 * i, state->from, state->to, state->length - (brightness_length * 30));
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

void reverse_animation_fade(animation_state_t *state)
{
  state->counter = state->length - state->counter;
  int from_tmp = state->from;
  state->from = state->to;
  state->to = from_tmp;
}

void light_fast_tick(void)
{
  static int hallway_brightness = 0;
  static int hallway_motion_sensor_brightness = 20000;
  static int hallway_animation = ANIMATION_NONE;
  static animation_state_t animation;
  static systime_t hallway_motion_sensor_on_time;

  int next_hallway_brightness = 0;

  if(!config_get_uint(CONFIG_HAS_LIGHT_CONTROL)) return;

  uint8_t hour = time_hour; // UTC

  if(hour >= 18 || hour <= 3)
  {
    if(hour <= 21)
    {
      // evening mode
      next_hallway_brightness = 2000;
      hallway_motion_sensor_brightness = 20000;
      pwm_set_dc(STAIRCASE_K20_1, 6000);
      pwm_set_dc(STAIRCASE_K20_2, 1500);
    }
    else
    {
      // night mode
      next_hallway_brightness = 70;
      hallway_motion_sensor_brightness = 5000;
      pwm_set_dc(STAIRCASE_K20_1, 350);
      pwm_set_dc(STAIRCASE_K20_2, 100);
    }
  }
  else {
    // day mode
    hallway_motion_sensor_brightness = 10;
    next_hallway_brightness = 10;
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
  if (hallway_motion_sensor_on_time + TIME_S2I(HALLWAY_MOTION_SENSOR_MAX_SECONDS) > chVTGetSystemTime())
  {
    next_hallway_brightness = hallway_motion_sensor_brightness;
  }

  // accept next_hallway_brightness, start animation if changed
  if (next_hallway_brightness != hallway_brightness)
  {
    if(hallway_animation == ANIMATION_NONE)
    {
      start_animation_fade(&animation, hallway_brightness, next_hallway_brightness, 300);
      hallway_animation = ANIMATION_FADE;
    }
    else if (hallway_animation == ANIMATION_FADE)
    {
      reverse_animation_fade(&animation);
    }
    hallway_brightness = next_hallway_brightness;
  }

  if(hallway_animation != ANIMATION_NONE)
  {
    // animation in progress
    uint16_t brightness[HALLWAY_LIGHTS];

    if (animate(brightness, HALLWAY_LIGHTS, &animation))
    {
      for(int i = 0; i < HALLWAY_LIGHTS; ++i)
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
      pwm_set_dc(HALLWAY_START + i, hallway_brightness);
    }
  }
}

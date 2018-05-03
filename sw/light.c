/*
 * light.c
 *
 *  Created on: 13.04.2018
 *      Author: matthias
 */


#include "config.h"
#include "drivers/pwm.h"
#include "time_data.h"
#include "drivers/analog.h" // for random data

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

/* GPIOB 5 is manual switch input, pull up */
#define HALLWAY_SWITCH_PORT GPIOB
#define HALLWAY_SWITCH_PAD 5
#define HALLWAY_SWITCH_MAX_SECONDS 120

#define ANIMATION_NONE 0
#define ANIMATION_FADE 1
#define ANIMATION_PLAY 2

static volatile uint32_t hallway_motion_sensor_trigger = 0;
static volatile uint32_t hallway_switch_trigger = 0;

typedef struct {
  int animation;
  int from;
  int to;
  int length;
  int counter;
  int lights_index[HALLWAY_LIGHTS];
} animation_state_t;

static void hallway_motion_sensor_callback(void* arg)
{
  hallway_motion_sensor_trigger = 1;
}

static void hallway_switch_callback(void* arg)
{
  hallway_switch_trigger = 1;
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
  palSetPadMode(HALLWAY_SWITCH_PORT, HALLWAY_SWITCH_PAD, PAL_MODE_INPUT_PULLDOWN);

  chSysLock();
  palEnablePadEventI(HALLWAY_MOTION_SENSOR_PORT, HALLWAY_MOTION_SENSOR_PAD, PAL_EVENT_MODE_RISING_EDGE);
  palSetPadCallbackI(HALLWAY_MOTION_SENSOR_PORT, HALLWAY_MOTION_SENSOR_PAD, hallway_motion_sensor_callback, NULL);
  palEnablePadEventI(HALLWAY_SWITCH_PORT, HALLWAY_SWITCH_PAD, PAL_EVENT_MODE_RISING_EDGE);
  palSetPadCallbackI(HALLWAY_SWITCH_PORT, HALLWAY_SWITCH_PAD, hallway_switch_callback, NULL);
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
    {
      if (state->counter >= state->length) return 0;
      int offset = state->length / 6;

      for(int i = 0; i < brightness_length; ++i)
      {
        brightness[state->lights_index[i]] = get_fade_val(state->counter - offset * i, state->from, state->to, state->length - (brightness_length * offset));
      }
      state->counter++;
      break;
    }

    case(ANIMATION_PLAY):
    {
      bool reverse = state->counter >= (state->length / 2);
      int fade_counter = state->counter % (state->length / 2);
      int fade_length = state->length / 2;

      int high = state->from * 1.4;
      int low = state->from * 0.6;
      for(int i = 0; i < brightness_length; ++i)
      {
        bool up = (i % 2) > 0;
        if (reverse) up = !up;
        int from = up ? low : high;
        int to = up ? high : low;
        brightness[state->lights_index[i]] = get_fade_val(fade_counter, from, to, fade_length);
      }
      state->counter++;
      state->counter %= state->length;

      brightness[2] = state->from;
      break;
    }

  }

  return 1;
}

void shuffle(int* array, size_t n)
{
  for(int i = 0; i < n; ++i)
  {
    int j = analog_get_vdda() % n;
    int temp = array[i];
    array[i] = array[j];
    array[j] = temp;
  }
}

void start_animation_fade(animation_state_t *state, int from, int to, int length)
{
  state->animation = ANIMATION_FADE;
  state->counter = 0;
  state->from = from;
  state->to = to;
  state->length = length;

  for(int i = 0; i < HALLWAY_LIGHTS; ++i)
  {
    state->lights_index[i] = i;
  }

  shuffle(state->lights_index, HALLWAY_LIGHTS);
}

void reverse_animation_fade(animation_state_t *state)
{
  state->counter = state->length - state->counter;
  int from_tmp = state->from;
  state->from = state->to;
  state->to = from_tmp;
}

void start_animation_play(animation_state_t *state, int from, int length)
{
  state->animation = ANIMATION_PLAY;
  state->counter = length / 4;
  state->from = from;
  state->length = length;

  for(int i = 0; i < HALLWAY_LIGHTS; ++i)
  {
    state->lights_index[i] = i;
  }
}


void light_fast_tick(void)
{
  static int hallway_brightness = 0;
  static int hallway_motion_sensor_brightness = 20000;
  static int hallway_animation = ANIMATION_NONE;
  static animation_state_t animation;
  static systime_t hallway_motion_sensor_on_time;
  static systime_t hallway_switch_on_time;

  int next_hallway_brightness = 0;

  if(!config_get_uint(CONFIG_HAS_LIGHT_CONTROL)) return;

  uint8_t hour = time_hour; // UTC

  if(!time_daylight)
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
    hallway_motion_sensor_brightness = 0;
    next_hallway_brightness = 0;
    pwm_set_dc(STAIRCASE_K20_1, 0);
    pwm_set_dc(STAIRCASE_K20_2, 0);
  }

  // check if motion sensor was triggered
  if(hallway_motion_sensor_trigger)
  {
    hallway_motion_sensor_trigger = 0;
    hallway_motion_sensor_on_time = chVTGetSystemTime();
  }

  if(hallway_switch_trigger)
  {
    hallway_switch_trigger = 0;
    // switch off if already triggered
    if (hallway_switch_on_time + TIME_S2I(HALLWAY_SWITCH_MAX_SECONDS) > chVTGetSystemTime())
    {
      hallway_switch_on_time -= TIME_S2I(HALLWAY_SWITCH_MAX_SECONDS);
    }
    hallway_switch_on_time = chVTGetSystemTime();
  }

  // override target brightness if motion sensor is on
  if (hallway_motion_sensor_on_time + TIME_S2I(HALLWAY_MOTION_SENSOR_MAX_SECONDS) > chVTGetSystemTime())
  {
    next_hallway_brightness = hallway_motion_sensor_brightness;
  }

  // override target brightness if switch is on
  if (hallway_switch_on_time + TIME_S2I(HALLWAY_SWITCH_MAX_SECONDS) > chVTGetSystemTime())
  {
    next_hallway_brightness = 40000;
  }

  // accept next_hallway_brightness, start animation if changed
  if (next_hallway_brightness != hallway_brightness)
  {
    if(hallway_animation == ANIMATION_FADE)
    {
      reverse_animation_fade(&animation);
    }
    else
    {
      start_animation_fade(&animation, hallway_brightness, next_hallway_brightness, 600);
      hallway_animation = ANIMATION_FADE;
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

    // start playing with brightness
    start_animation_play(&animation, hallway_brightness, 1024);
    hallway_animation = ANIMATION_PLAY;
  }
}

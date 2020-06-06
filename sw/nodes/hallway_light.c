#include "hal.h"
#include "../node.h"
#include "../config.h"
#include "../drivers/pwm.h"
#include "modules/time_data.h"
#include "modules/brightness_data.h"
#include "../drivers/analog.h" // for random data
#include "dsdl/homeautomation/Motion.h"

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
#define ANIMATION_PLAY 2

typedef struct {
  int animation;
  int from;
  int to;
  int length;
  int counter;
  int lights_index[HALLWAY_LIGHTS];
} animation_state_t;

static volatile systime_t hallway_motion_sensor_trigger_time;

static void hallway_motion_sensor_callback(void* arg)
{
  hallway_motion_sensor_trigger_time = chVTGetSystemTime();
}

static int get_fade_val(int step, int fade_from, int fade_to, int length)
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

static int animate(uint16_t *brightness, uint8_t brightness_length, animation_state_t *state)
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

static void shuffle(int* array, size_t n)
{
  for(int i = 0; i < n; ++i)
  {
    int j = analog_get_vdda() % n;
    int temp = array[i];
    array[i] = array[j];
    array[j] = temp;
  }
}

static void start_animation_fade(animation_state_t *state, int from, int to, int length)
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

static void reverse_animation_fade(animation_state_t *state)
{
  state->counter = state->length - state->counter;
  int from_tmp = state->from;
  state->from = state->to;
  state->to = from_tmp;
}

static void start_animation_play(animation_state_t *state, int from, int length)
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


void app_init(void)
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
  palSetPadCallbackI(HALLWAY_MOTION_SENSOR_PORT, HALLWAY_MOTION_SENSOR_PAD, hallway_motion_sensor_callback, NULL);
  chSysUnlock();
}

void app_fast_tick(void)
{
  static int hallway_brightness = 0;
  static int hallway_motion_sensor_brightness = 20000;
  static int hallway_animation = ANIMATION_NONE;
  static animation_state_t animation;

  int next_hallway_brightness = 0;

  if(!config_get_uint(CONFIG_HAS_LIGHT_CONTROL)) return;

  uint8_t hour = time_hour; // UTC

  if(brightness < 2)
  {
    // it's dark, turn light always on
    if(hour >= 6 && hour <= 22)
    {
      // evening mode
      next_hallway_brightness = 500;
      hallway_motion_sensor_brightness = 20000;
      pwm_set_dc(STAIRCASE_K20_1, 4000);
      pwm_set_dc(STAIRCASE_K20_2, 2000);
    }
    else
    {
      // night mode
      next_hallway_brightness = 70;
      hallway_motion_sensor_brightness = 2500;
      pwm_set_dc(STAIRCASE_K20_1, 500);
      pwm_set_dc(STAIRCASE_K20_2, 300);
    }
  }
  else if (brightness < 5) {
    // might be too dark in hallway still
    next_hallway_brightness = 0;
    hallway_motion_sensor_brightness = 20000;
    pwm_set_dc(STAIRCASE_K20_1, 0);
    pwm_set_dc(STAIRCASE_K20_2, 0);
  }
  else {
    // day mode
    hallway_motion_sensor_brightness = 0;
    next_hallway_brightness = 0;
    pwm_set_dc(STAIRCASE_K20_1, 0);
    pwm_set_dc(STAIRCASE_K20_2, 0);
  }

  // override target brightness if motion sensor is on
  if (hallway_motion_sensor_trigger_time + TIME_S2I(HALLWAY_MOTION_SENSOR_MAX_SECONDS) > chVTGetSystemTime())
  {
    next_hallway_brightness = hallway_motion_sensor_brightness;
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

static void send_motion_message()
{
    static uint8_t transfer_id = 0;
    uint8_t buf[HOMEAUTOMATION_MOTION_MAX_SIZE];
    homeautomation_Motion status;
    status.triggered = 1;
    homeautomation_Motion_encode(&status, buf);
    canardLockBroadcast(&canard,
      HOMEAUTOMATION_MOTION_SIGNATURE,
      HOMEAUTOMATION_MOTION_ID,
      &transfer_id,
      CANARD_TRANSFER_PRIORITY_LOW,
      buf,
      HOMEAUTOMATION_MOTION_MAX_SIZE
    );
}

void app_tick(void)
{
  static systime_t last_trigger_time = 0;
  if (last_trigger_time == 0) {
    // init
    last_trigger_time = chVTGetSystemTime();
  }

  if (hallway_motion_sensor_trigger_time > last_trigger_time) {
    send_motion_message();
    last_trigger_time = hallway_motion_sensor_trigger_time;
  }
}

void app_on_transfer_received(CanardInstance* ins, CanardRxTransfer* transfer)
{
  time_data_on_transfer_received(ins, transfer);
}

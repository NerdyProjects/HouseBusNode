#include <hal.h>
#include <ch.h>
#include "node.h"
#include "drivers/analog.h"
#include "config.h"
#include "conduction_sensor.h"
#include "time_data.h"

#define WATER_REFILL_PORT GPIOB
#define MAX_WATER_REFILL_SECONDS 5*60
#define TOP_CONDUCTION_SENSOR_INDEX 0

static uint16_t water_refill_pin = 0;
static uint8_t water_refill_present = 0;

systime_t last_water_refill_stop;
systime_t last_water_refill_start;
volatile uint8_t is_water_refill_on;

static void water_refill_start(void)
{
  last_water_refill_start = chVTGetSystemTime();
  is_water_refill_on = 1;
  palSetPad(WATER_REFILL_PORT, water_refill_pin);
}

static void water_refill_stop(void)
{
  last_water_refill_stop = chVTGetSystemTime();
  palClearPad(WATER_REFILL_PORT, water_refill_pin);
  is_water_refill_on = 0;
}

uint8_t water_refill_is_present(void)
{
  return water_refill_present;
}

void water_refill_init(void)
{
  int water_refill_control_pin;
  uint8_t valid;
  config_get(CONFIG_HAS_WATER_REFILL_OUTPUT_PIN, &water_refill_control_pin, &valid);
  /* Pins numbered 1..16 */
  if(valid < 4 || water_refill_control_pin < 1 || water_refill_control_pin > 16)
  {
    /* Water refill not activated or invalid configuration */
    return;
  }
  water_refill_pin = water_refill_control_pin - 1;
  water_refill_present = 1;

  palClearPad(WATER_REFILL_PORT, water_refill_pin);
  palSetPadMode(WATER_REFILL_PORT, water_refill_pin, PAL_MODE_OUTPUT_PUSHPULL);
}

void water_refill_tick(void)
{
  static uint8_t last_minute;

  if (!water_refill_is_present)
  {
    return;
  }

  uint8_t hour = time_hour; // UTC
  uint8_t minute = time_minute;

  bool minute_has_changed = last_minute != minute;
  last_minute = minute;

  int is_full = conduction_evaluate(TOP_CONDUCTION_SENSOR_INDEX, NULL);
  if (is_water_refill_on) {
    bool timeout = chVTGetSystemTime() > last_water_refill_start + TIME_S2I(MAX_WATER_REFILL_SECONDS);
    if (is_full || timeout)
    {
      water_refill_stop();
    }
  }
  else
  {
    bool is_hour_uneven = hour % 2;

    if (!is_full && is_hour_uneven && minute_has_changed && minute == 11)
    {
      water_refill_start();
    }
  }
}

/* retrieves the refill state and two status times:
 * stoppedForS: Time the refill is not running for in s since last stop
 * runningForS: Time the refill is running for in s since last start
 */
uint8_t water_refill_get_state(uint32_t *stoppedForS, uint16_t *runningForS)
{
  systime_t current = chVTGetSystemTime();
  uint8_t state;
  do {
    /* lazy synchronisation: data changes slowly, just read until we know we got a stable state.
     * Also, we might not need this when the reading thread always has lower priority than the writing thread. */
    state = is_water_refill_on;
    if(is_water_refill_on) {
      if(stoppedForS)
      {
        /* running: stop time is time before water refill was started */
        *stoppedForS = TIME_I2S(last_water_refill_start - last_water_refill_stop);
      }
      if(runningForS)
      { /* running: run time is time since water refill was started */
        *runningForS = TIME_I2S(current - last_water_refill_start);
      }
    } else {
      if(stoppedForS)
      {
        /* stopped: stop time is time since water refill was stopped */
        *stoppedForS = TIME_I2S(current - last_water_refill_stop);
      }
      if(runningForS)
      { /* stopped: run time is time between start and stop */
        *runningForS = TIME_I2S(last_water_refill_stop   - last_water_refill_start);
      }
    }
  } while(state != is_water_refill_on);
  return state;
}

/*
 * eventcount.c
 *
 *  Created on: 03.02.2018
 *      Author: matthias
 */

#include "hal.h"
#include "util.h"
#include "config.h"
#include "eventcount.h"

static stm32_gpio_t *Eventport;
static uint8_t Eventpin;
static systime_t EventStableMinST;
static systime_t LastStateChange;
static uint8_t LastState;
static uint8_t LastStableState;
static uint32_t EventCount;

void eventcount_init(void)
{
  int v;
  uint8_t valid;
  config_get(CONFIG_EVENTCOUNT_PORT, &v, &valid);
  if(valid < 1 || valid > 4 || (Eventport = portToGPIO(v)) == 0)
  {
    return;
  }
  config_get(CONFIG_EVENTCOUNT_PIN, &v, &valid);
  if(valid < 1 || valid > 4 || v > 15)
  {
    return;
  }
  Eventpin = v;
  config_get(CONFIG_EVENTCOUNT_MIN_STABLE_MS, &v, &valid);
  {
    if(valid < 1 || valid > 4)
    {
      return;
    }
  }
  EventStableMinST = MS2ST(v);
  palSetPadMode(Eventport, Eventpin, PAL_MODE_INPUT_PULLUP);
}

void eventcount_acquire(void)
{
  systime_t current = chVTGetSystemTime();
  uint8_t state = palReadPad(Eventport, Eventpin);
  if((state == LastState) && ((current - LastStateChange) > EventStableMinST))
  {
    /* stable state reached */
    if(state && !LastStableState)
    {
      EventCount++;
    }
    LastStableState = state;
  } else if(state != LastState)
  {
    LastState = state;
    LastStateChange = current;
  }
}

uint32_t eventcount_get_count(void)
{
  return EventCount;
}

uint8_t eventcount_is_present(void)
{
  return Eventport != 0;
}

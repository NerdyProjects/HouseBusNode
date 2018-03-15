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
#include <string.h>

static stm32_gpio_t *Eventport[EVENTCOUNT_PORTS];
static uint8_t Eventpin[EVENTCOUNT_PORTS];
static systime_t EventStableMinST[EVENTCOUNT_PORTS];
static systime_t LastStateChange[EVENTCOUNT_PORTS];
static uint8_t LastState[EVENTCOUNT_PORTS];
static uint8_t LastStableState[EVENTCOUNT_PORTS];
static uint32_t EventCount[EVENTCOUNT_PORTS];
static uint8_t EventcountPorts;

void eventcount_init(void)
{
  int v;
  uint8_t valid;
  EventcountPorts = 0;
  for(int i = 0; i < EVENTCOUNT_PORTS; ++i)
  {
    config_get(CONFIG_EVENTCOUNT0_PORT + 3*i, &v, &valid);
    if(valid < 1 || valid > 4 || (Eventport[i] = portToGPIO(v)) == 0)
    {
      break;
    }
    config_get(CONFIG_EVENTCOUNT0_PIN + 3*i, &v, &valid);
    if(valid < 1 || valid > 4 || v > 15)
    {
      break;
    }
    Eventpin[i] = v;
    config_get(CONFIG_EVENTCOUNT0_MIN_STABLE_MS + 3*i, &v, &valid);
    if(valid < 1 || valid > 4)
    {
      break;
    }
    EventStableMinST[i] = MS2ST(v);
    palSetPadMode(Eventport[i], Eventpin[i], PAL_MODE_INPUT_PULLUP);
    EventcountPorts++;
  }
}

void eventcount_acquire(void)
{
  systime_t current = chVTGetSystemTime();
  for(int i = 0; i < EventcountPorts; ++i)
  {
    uint8_t state = palReadPad(Eventport[i], Eventpin[i]);
    if(EventStableMinST[i] == 0 || ((state == LastState[i]) && ((current - LastStateChange[i]) > EventStableMinST[i])))
    {
      /* stable state reached */
      if(state && !LastStableState[i])
      {
        EventCount[i]++;
      }
      LastStableState[i] = state;
    } else if(state != LastState[i])
    {
      LastState[i] = state;
      LastStateChange[i] = current;
    }
  }
}

uint8_t eventcount_get_count(uint32_t *v)
{
  memcpy(v, EventCount, EventcountPorts * sizeof(uint32_t));
  return EventcountPorts;
}

uint8_t eventcount_is_present(void)
{
  return EventcountPorts;
}

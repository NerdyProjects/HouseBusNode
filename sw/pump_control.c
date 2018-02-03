/*
 * pump_control.c
 *
 *  Created on: 30.01.2018
 *      Author: matthias
 */

#include <hal.h>
#include <ch.h>
#include "node.h"
#include "drivers/analog.h"
#include "config.h"
#include "conduction_sensor.h"

#define PumpPort GPIOB
static uint16_t PumpPin = 0;
static uint8_t PumpPresent = 0;

#define CONDITION_LOCAL_CONDUCTION_SENSOR 0x10000

#define CONDITION_MODE_MASK 0x30000
#define CONDITION_LOCAL_CONDUCTION_SENSOR 0x10000
#define CONDITION_RUNNING_TIME_S 0x20000
#define CONDITION_TRUE 0x30000
#define CONDITION_VALUE_MASK 0x0FFFF
#define CONDITION_LOCAL_CONDUCTION_SENSOR 0x10000
#define CONDITION_RUNNING_TIME_S 0x20000
#define CONDITION_INVERSE 0x80000000

systime_t lastPumpStop;
systime_t lastPumpStart;
uint32_t pumpTrigger;
uint32_t pumpStopCond1;
uint32_t pumpStopCond2;
volatile uint8_t pumpState;

static int evaluateCondition(uint32_t condition)
{
  uint16_t val = condition & CONDITION_VALUE_MASK;
  uint8_t res = 0;
  switch(condition & CONDITION_MODE_MASK)
  {
  case CONDITION_LOCAL_CONDUCTION_SENSOR:
    res = conduction_evaluate(val, NULL);
    break;
  case CONDITION_RUNNING_TIME_S:
    res = (pumpState && ((chVTGetSystemTime() - lastPumpStart) >= S2ST(val)));
    break;
  case CONDITION_TRUE:
    res = 1;
    break;
  default:
    return 0;
    break;
  }
  return condition & CONDITION_INVERSE ? !res : res;
}

static void pumpStart(void)
{
  lastPumpStart = chVTGetSystemTime();
  pumpState = 1;
  palSetPad(GPIOB, PumpPin);
}

static void pumpStop(void)
{
  lastPumpStop = chVTGetSystemTime();
  palClearPad(GPIOB, PumpPin);
  pumpState = 0;
}

/* Pump controller thread.
 * Evaluates level sensor and takes corresponding action.
 */
static THD_WORKING_AREA(waPumpThread, 256);
static THD_FUNCTION(PumpThread, arg)
{
  (void) arg;
  chRegSetThreadName("Pump");
  while(node_getMode() != UAVCAN_NODE_MODE_OPERATIONAL)
  {
    chThdSleep(S2ST(5));
  }
  systime_t nextInvocation = chVTGetSystemTime();
  while (node_getMode() == UAVCAN_NODE_MODE_OPERATIONAL)
  {
    uint8_t trigger = evaluateCondition(pumpTrigger);
    uint8_t stop1 = evaluateCondition(pumpStopCond1);
    uint8_t stop2 = evaluateCondition(pumpStopCond2);
    if((stop1 || stop2) && trigger)
    {
      /* trigger and stop conditions active: Although possible we are precautious and don't retrigger but error out */
      pumpTrigger = 0;
      signalError(MODULE_PUMP_CONTROL);
    } else if(trigger && !pumpState) {
      pumpStart();
    } else if((stop1 || stop2) && pumpState)
    {
      pumpStop();
    }
    nextInvocation = chThdSleepUntilWindowed(nextInvocation, nextInvocation + MS2ST(500));
  }
  pumpStop();
}

/* retrieves the pump state and two status times:
 * stoppedForS: Time the pump is not running for in s since last stop
 * runningForS: Time the pump is running for in s since last start
 */
uint8_t pump_get_state(uint32_t *stoppedForS, uint16_t *runningForS)
{
  systime_t current = chVTGetSystemTime();
  uint8_t state;
  do {
    /* lazy synchronisation: data changes slowly, just read until we know we got a stable state.
     * Also, we might not need this when the reading thread always has lower priority than the writing thread. */
    state = pumpState;
    if(pumpState) {
      if(stoppedForS)
      {
        /* running: stop time is time before pump was started */
        *stoppedForS = ST2S(lastPumpStart - lastPumpStop);
      }
      if(runningForS)
      { /* running: run time is time since pump was started */
        *runningForS = ST2S(current - lastPumpStart);
      }
    } else {
      if(stoppedForS)
      {
        /* stopped: stop time is time since pump was stopped */
        *stoppedForS = ST2S(current - lastPumpStop);
      }
      if(runningForS)
      { /* stopped: run time is time between start and stop */
        *runningForS = ST2S(lastPumpStop   - lastPumpStart);
      }
    }
  } while(state != pumpState);
  return state;
}

uint8_t pump_is_present(void)
{
  /* TODO: proper present flag */
  return PumpPresent;
}

void pump_control_init(void)
{
  int pumpControlPin;
  uint8_t valid;
  config_get(CONFIG_HAS_PUMP_CONTROL_OUTPUT_PIN, &pumpControlPin, &valid);
  /* Pins numbered 1..16 */
  if(valid < 4 || pumpControlPin < 1 || pumpControlPin > 16)
  {
    /* Pump controller not activated or invalid configuration */
    return;
  }
  config_get(CONFIG_PUMP_TRIGGER_SOURCE, &pumpTrigger, NULL);
  config_get(CONFIG_PUMP_STOP_CONDITION_1, &pumpStopCond1, NULL);
  config_get(CONFIG_PUMP_STOP_CONDITION_2, &pumpStopCond2, NULL);
  PumpPin = pumpControlPin - 1;
  PumpPresent = 1;

  palClearPad(PumpPort, PumpPin);
  palSetPadMode(PumpPort, PumpPin, PAL_MODE_OUTPUT_PUSHPULL);
  chThdCreateStatic(waPumpThread, sizeof(waPumpThread), HIGHPRIO, PumpThread,
        NULL);
}

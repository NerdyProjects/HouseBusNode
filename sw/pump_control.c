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
static int32_t PumpPin = 0;

#define CONDITION_LOCAL_CONDUCTION_SENSOR 0x10000

#define CONDITION_MODE_MASK 0x30000
#define CONDITION_VALUE_MASK 0x0FFFF
#define CONDITION_LOCAL_CONDUCTION_SENSOR 0x10000
#define CONDITION_RUNNING_TIME_S 0x20000

systime_t lastPumpStop;
systime_t lastPumpStart;
uint32_t pumpTrigger;
uint32_t pumpStopCond1;
uint32_t pumpStopCond2;
uint8_t pumpState;

static int evaluateCondition(uint32_t condition)
{
  uint16_t val = condition & CONDITION_VALUE_MASK;
  switch(condition & CONDITION_MODE_MASK)
  {
  case CONDITION_LOCAL_CONDUCTION_SENSOR:
    return conduction_evaluate(val, NULL);
    break;
  case CONDITION_RUNNING_TIME_S:
    return (pumpState && ((chVTGetSystemTime() - lastPumpStart) >= S2ST(val)));
    break;
  default:
    return false;
    break;
  }
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
  systime_t nextInvocation = chVTGetSystemTime();
  while (true)
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
}

void pump_control_init(void)
{
  int pumpControlPin;
  uint8_t valid;
  config_get(CONFIG_HAS_PUMP_CONTROL_OUTPUT_PIN, &pumpControlPin, &valid);
  if(valid < 4 || pumpControlPin < 0 || pumpControlPin > 15)
  {
    /* Pump controller not activated or invalid configuration */
    return;
  }
  config_get(CONFIG_PUMP_TRIGGER_SOURCE, &pumpTrigger, NULL);
  config_get(CONFIG_PUMP_STOP_CONDITION_1, &pumpStopCond1, NULL);
  config_get(CONFIG_PUMP_STOP_CONDITION_2, &pumpStopCond2, NULL);
  PumpPin = pumpControlPin;

  palClearPad(PumpPort, PumpPin);
  palSetPadMode(PumpPort, PumpPin, PAL_MODE_OUTPUT_PUSHPULL);
  chThdCreateStatic(waPumpThread, sizeof(waPumpThread), HIGHPRIO, PumpThread,
        NULL);
}

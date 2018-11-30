/*
 * temperature_receiver.c
 *
 *  Created on: 30.11.2018
 *      Author: matthias
 */

#include "temperature_receiver.h"

static uint8_t targetNodeId;
static int16_t temperature;
static systime_t temperatureReceivedAt;
static uint8_t temperatureValid;

/* to catch overflow behaviour and some sanity, always mark data as invalid above this age */
#define MAX_VALID_AGE TIME_S2I(86400)

void setReceiverTarget(uint8_t node_id)
{
  targetNodeId = node_id;
}

void on_environment_data(CanardInstance* ins, CanardRxTransfer* transfer)
{
  int32_t temp;
  if(transfer->payload_len < HOMEAUTOMATION_ENVIRONMENT_MESSAGE_SIZE) {
    /* invalid payload */
    return;
  }
  if (transfer->source_node_id != targetNodeId)
  {
    return;
  }

  canardDecodeScalar(transfer, 2, 19, 0, &temp);
  temperature = temp;
  temperatureReceivedAt = transfer->timestamp;
  temperatureValid = 1;
}

int16_t getTargetTemperature(sysinterval_t *ageOut)
{
  sysinterval_t age = chVTTimeElapsedSinceX(temperatureReceivedAt);
  if(age > MAX_VALID_AGE || !temperatureValid)
  {
    temperatureValid = 0;
    age = -1;
  }
  if(ageOut)
  {
    *ageOut = age;
  }
  return temperature;
}

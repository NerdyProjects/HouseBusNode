/*
 * temperature_receiver.h
 *
 *  Created on: 30.11.2018
 *      Author: matthias
 */

#ifndef MODULES_TEMPERATURE_RECEIVER_H_
#define MODULES_TEMPERATURE_RECEIVER_H_

#include "node.h"

void on_environment_data(CanardInstance* ins, CanardRxTransfer* transfer);
void setReceiverTarget(uint8_t node_id);
int16_t getTargetTemperature(sysinterval_t *ageOut);

REGISTER_TRANSFER(CanardTransferTypeBroadcast, HOMEAUTOMATION_ENVIRONMENT_DATA_TYPE_ID, HOMEAUTOMATION_ENVIRONMENT_DATA_TYPE_SIGNATURE, on_environment_data)


#endif /* MODULES_TEMPERATURE_RECEIVER_H_ */

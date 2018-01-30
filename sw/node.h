/*
 * node.h
 *
 *  Created on: 13.01.2018
 *      Author: matthias
 */

#ifndef NODE_H_
#define NODE_H_

#include "canard.h"

#define MODULE_PUMP_CONTROL 0x10000

void node_init(void);
void node_tx_request(void);
void requestNodeRestart(void);
void signalError(uint32_t code);
extern CanardInstance canard;
void node_setMode(uint8_t mode);

#endif /* NODE_H_ */

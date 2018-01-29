/*
 * node.h
 *
 *  Created on: 13.01.2018
 *      Author: matthias
 */

#ifndef NODE_H_
#define NODE_H_

#include "canard.h"

void node_init(void);
void node_tx_request(void);
void requestNodeRestart(void);
extern CanardInstance canard;

#endif /* NODE_H_ */

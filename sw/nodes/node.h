/*
 * node.h
 *
 *  Created on: 08.09.2018
 *      Author: matthias
 */

#ifndef NODES_NODE_H_
#define NODES_NODE_H_
#include "../node.h"

void app_fast_tick(void);
void app_init(void);
void app_tick(void);
void app_config_update(void);
void app_on_transfer_received(CanardInstance* ins, CanardRxTransfer* transfer);


#endif /* NODES_NODE_H_ */

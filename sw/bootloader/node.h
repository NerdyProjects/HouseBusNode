/*
 * node.h
 *
 *  Created on: 13.01.2018
 *      Author: matthias
 */

#ifndef NODE_H_
#define NODE_H_

#include "canard.h"
#include "uavcan.h"

void node_init(uint8_t node_id);
void node_tx_request(void);
void requestNodeRestart(void);
extern CanardInstance canard;
int canardLockBroadcast(CanardInstance* ins,            ///< Library instance
                    uint64_t data_type_signature,   ///< See above
                    uint16_t data_type_id,          ///< Refer to the specification
                    uint8_t* inout_transfer_id,     ///< Pointer to a persistent variable containing the transfer ID
                    uint8_t priority,               ///< Refer to definitions CANARD_TRANSFER_PRIORITY_*
                    const void* payload,            ///< Transfer payload
                    uint16_t payload_len);          ///< Length of the above, in bytes

typedef void (*OnTransferReceivedCB)(CanardInstance* ins, CanardRxTransfer* transfer);

#define REGISTER_TRANSFER(...)
#endif /* NODE_H_ */

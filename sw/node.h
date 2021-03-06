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

void node_init(void);
void node_tx_request(void);
void requestNodeRestart(void);
void signalError(uint32_t code);
extern CanardInstance canard;
void node_setMode(uint8_t mode);
uint8_t node_getMode(void);
typedef enum UAVCAN_LOG_LEVEL
{
  LOG_LEVEL_DEBUG = 0,
  LOG_LEVEL_INFO = 1,
  LOG_LEVEL_WARNING = 2,
  LOG_LEVEL_ERROR = 3
} UavcanLogLevel;

void node_debug(uint8_t loglevel, const char *source, const char *msg);
void node_debug_int(uint8_t loglevel, const char *source, int32_t number);
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

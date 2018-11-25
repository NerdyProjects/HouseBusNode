#ifndef TIME_DATA_H_
#define TIME_DATA_H_

#include "node.h"

void on_time_data(CanardInstance* ins, CanardRxTransfer* transfer);
extern volatile uint8_t time_hour;
extern volatile uint8_t time_minute;
extern volatile bool time_daylight;

REGISTER_TRANSFER(CanardTransferTypeBroadcast, HOMEAUTOMATION_TIME_DATA_TYPE_ID, HOMEAUTOMATION_TIME_DATA_TYPE_SIGNATURE, on_time_data)

#endif /* TIME_DATA_H_ */

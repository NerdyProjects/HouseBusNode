#ifndef TIME_DATA_H_
#define TIME_DATA_H_

#include "node.h"

void on_time_data(CanardRxTransfer* transfer);
extern volatile uint8_t time_hour;
extern volatile uint8_t time_minute;

#endif /* TIME_DATA_H_ */

#ifndef BRIGHTNESS_DATA_H_
#define BRIGHTNESS_DATA_H_

#include "node.h"
#include "homeautomation/BathroomStatus.h"

void on_bathroom_data(CanardInstance* ins, CanardRxTransfer* transfer);
extern volatile uint8_t brightness;

#endif /* BRIGHTNESS_DATA_H_ */

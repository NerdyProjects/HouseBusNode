/*
 * bme280_node.h
 *
 *  Created on: 21.01.2018
 *      Author: matthias
 */

#ifndef BME280_NODE_H_
#define BME280_NODE_H_

#include "drivers/bme280_defs.h"

void bme280_app_init(void);
void bme280_app_read(void);

extern uint8_t bme_presence;         /* LSB marks presence of primary/secondary BME I2C, 1 << 0, 1 << 1 */
extern int16_t BMECentiTemperature[2];   /* -327(.)68 - 327(.)68 degrees */
extern uint32_t BMEMilliHumidity[2];     /* 0 - 100(.)000 % */
extern uint32_t BMECentiPressure[2];     /* 300(.)00-1100(.)00 mBar  */



#endif /* BME280_NODE_H_ */

/*
 * bme280_node.h
 *
 *  Created on: 21.01.2018
 *      Author: matthias
 */

#ifndef BME280_NODE_H_
#define BME280_NODE_H_

#include "drivers/bme280_defs.h"

void bme280_node_init(void);
int bme280_node_read(struct bme280_data *data);
int bme280_is_present(void);


#endif /* BME280_NODE_H_ */

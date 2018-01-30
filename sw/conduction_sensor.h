/*
 * conduction_sensor.h
 *
 *  Created on: 30.01.2018
 *      Author: matthias
 */

#ifndef CONDUCTION_SENSOR_H_
#define CONDUCTION_SENSOR_H_

void conduction_init(void);
void conduction_acquire(void);
uint8_t conduction_evaluate(uint8_t sensor, uint8_t *quality);


#endif /* CONDUCTION_SENSOR_H_ */

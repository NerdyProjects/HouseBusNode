/*
 * pump_control.h
 *
 *  Created on: 30.01.2018
 *      Author: matthias
 */

#ifndef PUMP_CONTROL_H_
#define PUMP_CONTROL_H_

void pump_control_init(void);
uint8_t pump_is_present(void);
uint8_t pump_get_state(uint32_t *stoppedForS, uint16_t *runningForS);

#endif /* PUMP_CONTROL_H_ */

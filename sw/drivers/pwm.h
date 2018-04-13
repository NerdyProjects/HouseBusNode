/*
 * pwm.h
 *
 *  Created on: 13.04.2018
 *      Author: matthias
 */

#ifndef DRIVERS_PWM_H_
#define DRIVERS_PWM_H_

void pwm_init(void);
void pwm_set_dc(uint8_t channel, uint16_t dc);

#endif /* DRIVERS_PWM_H_ */

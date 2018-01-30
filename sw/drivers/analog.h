/*
 * analog.h
 *
 *  Created on: 28.01.2018
 *      Author: matthias
 */

#ifndef DRIVERS_ANALOG_H_
#define DRIVERS_ANALOG_H_

void analog_init(void);
int analog_get_internal_ts(void);
int analog_get_vdda(void);
int analog_meassure_conduction(void);
int analog_conduction_closed(uint8_t sensor);

/* access to ADC data, updated with ~3 kHz, IIR-filtered with alpha = 0.125 */
extern volatile uint16_t adc_smp_filtered[18];
#endif /* DRIVERS_ANALOG_H_ */

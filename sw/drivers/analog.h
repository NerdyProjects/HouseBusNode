/*
 * analog.h
 *
 *  Created on: 28.01.2018
 *      Author: matthias
 */

#ifndef DRIVERS_ANALOG_H_
#define DRIVERS_ANALOG_H_

void analog_filter_reset(uint8_t channel);
void analog_init(void);
int analog_get_internal_ts(void);
int analog_get_vdda(void);

#define ANALOG_MAX (16*4095)
#define ANALOG_CHANNELS 18

/* access to ADC data, updated with ~3 kHz, IIR-filtered with alpha = 0.0625 */
extern volatile uint16_t adc_smp_filtered[18];
#endif /* DRIVERS_ANALOG_H_ */

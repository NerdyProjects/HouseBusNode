/*
 * dimmer.h
 */

#ifndef DIMMER_H_
#define DIMMER_H_

void dimmer_init(void);
void dimmer_read_config(void);
void dimmer_tick(void);
void dimmer_set_dc(uint8_t dc);
uint8_t dimmer_is_present(void);

#endif /* DIMMER_H_ */

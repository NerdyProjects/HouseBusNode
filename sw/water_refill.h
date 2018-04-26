#ifndef WATER_REFILL_H_
#define WATER_REFILL_H_

void water_refill_init(void);
uint8_t water_refill_is_present(void);
uint8_t water_refill_get_state(uint32_t *stoppedForS, uint16_t *runningForS);

#endif /* WATER_REFILL_H_ */

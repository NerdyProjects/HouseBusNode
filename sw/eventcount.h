/*
 * eventcount.h
 *
 *  Created on: 03.02.2018
 *      Author: matthias
 */

#ifndef EVENTCOUNT_H_
#define EVENTCOUNT_H_

void eventcount_init(void);
void eventcount_acquire(void);
uint32_t eventcount_get_count(void);
uint8_t eventcount_is_present(void);

#endif /* EVENTCOUNT_H_ */

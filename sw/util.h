/*
 * util.h
 *
 *  Created on: 13.01.2018
 *      Author: matthias
 */

#ifndef UTIL_H_
#define UTIL_H_

#include <ch.h>
#include <hal.h>

#if 1
#define DEBUG(...)
#define ERROR(...)
#else
#define DEBUG(...) chprintf((BaseSequentialStream *) &STDOUT_SD, __VA_ARGS__)
#define ERROR(...) chprintf((BaseSequentialStream *) &STDOUT_SD, __VA_ARGS__)
#endif

#define getMonotonicTimestamp(x) chVTGetSystemTimeX()

#define CANARD_ASSERT(x) util_assert(x)

#define util_assert(x)

/*static void util_assert(int x) {
  if(!x) {
    while(1)
      ;
  }
} */

static inline stm32_gpio_t* portToGPIO(uint8_t port)
{
  switch(port)
  {
  case 1:
    return GPIOA;
  case 2:
    return GPIOB;
  case 3:
    return GPIOC;
  case 4:
    return GPIOD;
  case 5:
    return GPIOE;
  case 6:
    return GPIOF;
  default:
    return 0;
  }
}


#endif /* UTIL_H_ */

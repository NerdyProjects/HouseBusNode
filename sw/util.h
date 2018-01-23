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
#include "chprintf.h"

#ifdef NDEBUG
#define DEBUG(...)
#else
#define DEBUG(...) chprintf((BaseSequentialStream *) &STDOUT_SD, __VA_ARGS__)
#endif
#define ERROR(...) chprintf((BaseSequentialStream *) &STDOUT_SD, __VA_ARGS__)

static inline uint64_t ST2US_64(systime_t st)
{
  return ((((uint64_t)st * 1000000ULL)
        + (uint64_t) CH_CFG_ST_FREQUENCY - 1ULL)
        / (uint64_t) CH_CFG_ST_FREQUENCY);
}

static inline uint64_t getMonotonicTimestampUSec(void)
{
  systime_t currentTime = chVTGetSystemTime();
  return (uint64_t)currentTime * (uint64_t)(1000000ULL/CH_CFG_ST_FREQUENCY);
}

#define CANARD_ASSERT(x) util_assert(x)

static void util_assert(int x) {
  if(!x) {
    while(1)
      ;
  }
}



#endif /* UTIL_H_ */

/*
 * util.h
 *
 *  Created on: 13.01.2018
 *      Author: matthias
 */

#ifndef UTIL_H_
#define UTIL_H_

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
  return ST2US_64(currentTime);
}


#endif /* UTIL_H_ */

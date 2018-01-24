/*
 * i2c.c
 *
 *  Created on: 24.01.2018
 *      Author: matthias
 */

#include "hal.h"
static const I2CConfig i2cconfig = {
  STM32_TIMINGR_PRESC(0U) |
  STM32_TIMINGR_SCLDEL(3U) | STM32_TIMINGR_SDADEL(1U) |
  STM32_TIMINGR_SCLH(3U)  | STM32_TIMINGR_SCLL(9U),
  0,
  0
};

void i2c_init(void)
{
  i2cStart(&I2CD1, &i2cconfig);
}

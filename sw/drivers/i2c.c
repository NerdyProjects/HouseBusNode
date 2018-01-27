/*
 * i2c.c
 *
 *  Created on: 24.01.2018
 *      Author: matthias
 */

#include "hal.h"
/* 100 kHz @ 8 MHz base clock */
static const I2CConfig i2cconfig = {
  STM32_TIMINGR_PRESC(1U) |
  STM32_TIMINGR_SCLDEL(4U) | STM32_TIMINGR_SDADEL(2U) |
  STM32_TIMINGR_SCLH(0x0FU)  | STM32_TIMINGR_SCLL(0x13U),
  0,
  0
};

/* recovers and inits the I2C bus */
void i2c_init(void)
{
  uint8_t sda_high_cycles = 0;
  palSetPadMode(GPIOB, GPIOB_I2C1_SCL, PAL_MODE_OUTPUT_OPENDRAIN);
  palSetPad(GPIOB, GPIOB_I2C1_SDA);
  palSetPadMode(GPIOB, GPIOB_I2C1_SDA, PAL_MODE_OUTPUT_OPENDRAIN);
  palClearPad(GPIOB, GPIOB_I2C1_SCL);
  chThdSleep(US2ST(5));
  while(!palReadPad(GPIOB, GPIOB_I2C1_SDA))
  {
    palSetPad(GPIOB, GPIOB_I2C1_SCL);
    chThdSleep(US2ST(5));
    palClearPad(GPIOB, GPIOB_I2C1_SCL);
    chThdSleep(US2ST(5));
  }
  /* STOP condition */
  palClearPad(GPIOB, GPIOB_I2C1_SDA);
  palSetPad(GPIOB, GPIOB_I2C1_SCL);
  chThdSleep(US2ST(5));
  palSetPad(GPIOB, GPIOB_I2C1_SDA);
  chThdSleep(US2ST(5));
  /* Send SCL cycles until SDA is high for 10 consecutive cycles */
  do {
    palSetPad(GPIOB, GPIOB_I2C1_SCL);
    if(palReadPad(GPIOB, GPIOB_I2C1_SDA))
    {
      sda_high_cycles++;
    } else
    {
      sda_high_cycles = 0;
    }
    chThdSleep(US2ST(5));
    palClearPad(GPIOB, GPIOB_I2C1_SCL);
    chThdSleep(US2ST(5));
  } while(sda_high_cycles < 10);
  /* send a STOP condition afterwards */
  palClearPad(GPIOB, GPIOB_I2C1_SDA);
  chThdSleep(US2ST(5));
  palSetPad(GPIOB, GPIOB_I2C1_SCL);
  chThdSleep(US2ST(5));
  palSetPad(GPIOB, GPIOB_I2C1_SDA);
  chThdSleep(US2ST(5));
  /* Start the I2C peripheral */
  palSetPadMode(GPIOB, GPIOB_I2C1_SCL, PAL_MODE_ALTERNATE(1));
  palSetPadMode(GPIOB, GPIOB_I2C1_SDA, PAL_MODE_ALTERNATE(1));
  i2cStart(&I2CD1, &i2cconfig);
  /* give I2C devices some time to recover from internal action */
  chThdSleep(MS2ST(10));
}

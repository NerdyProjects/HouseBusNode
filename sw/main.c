/*
 ChibiOS - Copyright (C) 2006..2016 Giovanni Di Sirio

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "node.h"
#include "bme280_node.h"

#ifndef BOOTLOADER
static const I2CConfig i2cconfig = {
  STM32_TIMINGR_PRESC(0U) |
  STM32_TIMINGR_SCLDEL(3U) | STM32_TIMINGR_SDADEL(1U) |
  STM32_TIMINGR_SCLH(3U)  | STM32_TIMINGR_SCLL(9U),
  0,
  0
};

#endif

/*
 * Application entry point.
 */
int main(void)
{

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();


  /*
   * Activates the serial driver 2 using the driver default configuration.
   */
  sdStart(&SD1, NULL);
#ifndef BOOTLOADER
  i2cStart(&I2CD1, &i2cconfig);
  bme280_node_init();
#endif
  chprintf((BaseSequentialStream *) &STDOUT_SD, "SYSCLK=%u\r\n", STM32_SYSCLK);

  node_init();


  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop and check the button state.
   */
  while (true)
  {
    chThdSleepMilliseconds(500);
  }
}


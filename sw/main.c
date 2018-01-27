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
#include "drivers/i2c.h"
#include "config.h"
#ifdef BOOTLOADER
#include "bootloader.h"
#endif

static const WDGConfig wdgconfig = {
    7, 164, 0x0FFF
};

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
  DBGMCU->APB1FZ |= DBGMCU_APB1_FZ_DBG_IWDG_STOP | DBGMCU_APB1_FZ_DBG_WWDG_STOP;
  wdgInit();
//  wdgStart(&WDGD1, &wdgconfig);

  /*
   * Activates the serial driver 2 using the driver default configuration.
   */
  sdStart(&SD1, NULL);
  wdgReset(&WDGD1);
  i2c_init();
  if(config_init(&I2CD1) < 0)
  {
    ERROR("configuration init failed\n");
  }
#ifndef BOOTLOADER
  wdgReset(&WDGD1);
  bme280_node_init();
  wdgReset(&WDGD1);
#endif
  chprintf((BaseSequentialStream *) &STDOUT_SD, "SYSCLK=%u\r\n", STM32_SYSCLK);
  wdgReset(&WDGD1);
  node_init();

  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop and check the button state.
   */
#ifdef BOOTLOADER
  bootloader_loop();
#endif
  while (true)
  {
    chThdSleepMilliseconds(500);
  }
}


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
#include "node.h"
#include "uavcan.h"
#include "bootloader.h"
#include "../bootloader_interface.h"

/* by default, boot application after this timeout */
#define BOOTLOADER_TIMEOUT TIME_S2I(10)
/* after a command has been received, use this timeout to boot application */
#define BOOTLOADER_COMMAND_TIMEOUT TIME_S2I(60)

#define DEFAULT_NODE_ID 126

extern volatile uint8_t FirmwareUpdate;

static const WDGConfig wdgconfig = {
    7, 0x0FFF, 0x0FFF
};

/*
 * Application entry point.
 */
int main(void)
{
  uint8_t node_id = DEFAULT_NODE_ID;

  halInit();
  chSysInit();
  wdgInit();
  RCC->APB2ENR |= RCC_APB2ENR_DBGMCUEN; //enable MCU debug module clock
  DBGMCU->APB1FZ |= DBGMCU_APB1_FZ_DBG_IWDG_STOP | DBGMCU_APB1_FZ_DBG_WWDG_STOP;
  wdgStart(&WDGD1, &wdgconfig);
  systime_t boot_application_at = chVTGetSystemTime() + BOOTLOADER_TIMEOUT;
  if(bootloader_interface.magic == BOOTLOADER_INTERFACE_VALID_MAGIC &&
     bootloader_interface.node_id)
  {
    node_id = bootloader_interface.node_id;
    if(bootloader_interface.request_from_node_id && bootloader_interface.request_file_name_length)
    {
      FirmwareUpdate = 1;
    }
  }
  node_init(node_id);

  wdgReset(&WDGD1);

  while(boot_application_at > chVTGetSystemTime())
  {
    chThdSleep(TIME_MS2I(500));
    if(FirmwareUpdate)
    {
      /* delay boot until 2 seconds after firmware update done */
      boot_application_at = chVTGetSystemTime() + TIME_S2I(2);
    }
    wdgReset(&WDGD1);
  }
  boot_application();
}


/*
 * bootloader.c
 *
 *  Created on: 27.01.2018
 *      Author: matthias
 */
#include <string.h>
#include "hal.h"
#include "ch.h"
#include "../bootloader_interface.h"

extern uint32_t __application_flash_base__;
uint32_t __attribute__(( section (".vtors_ram"))) vector_table[47];

/* Boots the application by
 *
 * preparing the new interrupt vecotor table,
 * disabling all peripherals and
 * jumping to the application code.
 *
 * The watchdog is intentionally left running so a broken software will lead back
 * to the bootloader.
 */
void boot_application(void)
{
  __disable_irq();
  /* first, disable all peripherals as we don't need anything anymore */
  rccDisableAHB(~0x00000014);
  rccDisableAPB1(0xFFFFFFFF);
  rccDisableAPB2(0xFFFFFFFF);
  rccResetCAN1();
  rccResetI2C1();
  rccResetTIM2();
  rccResetTIM3();
  rccResetUSART1();
  /* disable all interrupt sources*/
  for(int i = 0; i < 32; ++i)
  {
    NVIC_DisableIRQ(i);
  }

  uint32_t *application_flash = &__application_flash_base__;
  /* load the application vector table into SRAM */
  memcpy(vector_table, &__application_flash_base__, sizeof(vector_table));
  /* Remap SRAM to 0x0 so interrupt vectors are read from SRAM */
  SYSCFG->CFGR1 = SYSCFG_CFGR1_MEM_MODE;
  /* actually, ChibiOs does this as well if configured, but better safe than sorry... */
  __set_MSP(__application_flash_base__);
  /* Jump to RESET of application, make sure to be in thumb mode */
  uint32_t jump_target = application_flash[1] | 1;
  asm("BX %0" : : "r"(jump_target));
}

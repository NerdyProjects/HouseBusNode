/*
 * bootloader.c
 *
 *  Created on: 27.01.2018
 *      Author: matthias
 */
#include <string.h>
#include "hal.h"
#include "ch.h"
#include "bootloader_interface.h"

extern uint32_t __application_flash_base__;
uint32_t __attribute__(( section (".vtors_ram"))) vector_table[47];
extern volatile uint8_t FirmwareUpdate;

/* by default, boot application after this timeout */
#define BOOTLOADER_TIMEOUT S2ST(10)
/* after a command has been received, use this timeout to boot application */
#define BOOTLOADER_COMMAND_TIMEOUT S2ST(60)

/* simple flag to pronlongue the bootloader timeout */
volatile uint8_t command_executed = 0;

/* Boots the application by
 *
 * preparing the new interrupt vecotor table,
 * disabling all peripherals and
 * jumping to the application code.
 *
 * The watchdog is intentionally left running so a broken software will lead back
 * to the bootloader.
 */
static void boot_application(void)
{
  __disable_irq();
  /* first, disable all peripherals as we don't need anything anymore */
  rccDisableAHB(~0x00000014, 0);
  rccDisableAPB1(0xFFFFFFFF, 0);
  rccDisableAPB2(0xFFFFFFFF, 0);
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


void bootloader_command_executed(void)
{
  command_executed = 1;
}

void bootloader_loop(void)
{
  systime_t boot_application_at = chVTGetSystemTime() + BOOTLOADER_TIMEOUT;
  if(bootloader_interface.magic == BOOTLOADER_INTERFACE_VALID_MAGIC &&
     bootloader_interface.request_from_node_id &&
     bootloader_interface.request_file_name_length)
  {
    FirmwareUpdate = 1;
    command_executed = 1;
    /* Invalidate data from now on */
    bootloader_interface.magic = 0;
  }
  while(boot_application_at > chVTGetSystemTime())
  {
    if(command_executed)
    {
      command_executed = 0;
      boot_application_at =  chVTGetSystemTime() + BOOTLOADER_COMMAND_TIMEOUT;
    }
    chThdSleep(MS2ST(500));
  }
  boot_application();
}


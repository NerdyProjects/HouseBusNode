/*
 * firmware_update.c
 *
 *  Created on: 24.01.2018
 *      Author: matthias
 */

#include "canard.h"
#include "drivers/flash.h"
#include "bootloader_interface.h"

static uint32_t file_request_ptr = 0;
static uint32_t file_received_ptr = 0;
static uint8_t firmware_buffer[FLASH_PAGE_SIZE];

extern uint32_t __application_flash_base__;

const static uint32_t firmware_max_size = FLASH_TOT_SIZE - __application_flash_base__;



void onFileRead(CanardInstance* ins, CanardRxTransfer* transfer)
{

}

void processFirmwareUpdate(void)
{
  if(file_request_ptr == file_received_ptr)
  {
    /* received all data, so request new or write */
  }


}

/*
 * firmware_update.c
 *
 *  Created on: 24.01.2018
 *      Author: matthias
 */

#include <string.h>
#include "canard.h"
#include "../drivers/flash.h"
#include "../bootloader_interface.h"
#include "firmware_update.h"
#include "uavcan.h"
#include "node.h"



static uint32_t file_request_ptr = 0;
static uint8_t file_request_pending = 0;
static uint8_t file_eof = 0;

static uint8_t firmware_buffer[FLASH_PAGE_SIZE];
static uint32_t firmware_buffer_ptr = 0;
static uint32_t firmware_write_offset = 0;


extern uint32_t __application_flash_base__;

void onFileRead(CanardInstance* ins, CanardRxTransfer* transfer)
{
  int16_t error;
  int16_t chunk_length = transfer->payload_len - 2;
  if(!file_request_pending)
  {
    ERROR("FileRead response wo request\n");
    return;
  }
  if(chunk_length < 0 || chunk_length > 256)
  {
    ERROR("FileRead resp invalid size\n");
    return;
  }
  canardDecodeScalar(transfer, 0, 16, 1, &error);
  for(int i = 0; i < chunk_length; ++i)
  {
    canardDecodeScalar(transfer, 16+8*i, 8, 0, &firmware_buffer[firmware_buffer_ptr+i]);
  }
  firmware_buffer_ptr += chunk_length;
  file_request_pending = 0;
  if(chunk_length != 256)
  {
    file_eof = 1;
  }
}

static void requestFileRead(CanardInstance* ins, uint32_t offset)
{
  static uint8_t transfer_id = 0;
  uint8_t request[UAVCAN_FILE_READ_REQUEST_MAX_SIZE];
  uint64_t offset64 = offset;
  uint16_t size = 40/8 + bootloader_interface.request_file_name_length;
  canardEncodeScalar(request, 0, 40, &offset64);
  memcpy(request + 40/8, bootloader_interface.request_file_name, bootloader_interface.request_file_name_length);
  const int bc_res = canardRequestOrRespond(ins,
      bootloader_interface.request_from_node_id,
      UAVCAN_FILE_READ_DATA_TYPE_SIGNATURE,
      UAVCAN_FILE_READ_DATA_TYPE_ID, &transfer_id,
      CANARD_TRANSFER_PRIORITY_LOW, CanardRequest, request, size);
  if (bc_res <= 0)
  {
    ERROR("file req sending error %d\n", bc_res);
  }
  node_tx_request();
}

static void initGlobals(void)
{
  firmware_buffer_ptr = 0;
  file_eof = 0;
  file_request_ptr = 0;
  firmware_write_offset = 0;
}

int processFirmwareUpdate(CanardInstance* ins)
{
  if(!file_request_pending)
  {
    /* received all requested data, so request new or write */
    if(file_eof || firmware_buffer_ptr == FLASH_PAGE_SIZE)
    {
      /* enough data to write -> write */
      void *flash_write = (void *)((uint32_t)&__application_flash_base__ + firmware_write_offset);
      DEBUG("Updating flash at %x (len %d):", (uint32_t)flash_write, firmware_buffer_ptr);
      if(!flash_verify_block(flash_write, firmware_buffer, firmware_buffer_ptr))
      {
        /* Content mismatch - erase and write */
        flash_erase_page(flash_write);
        DEBUG(" (er)");
        flash_write_block(flash_write, firmware_buffer, firmware_buffer_ptr);
        DEBUG(" (wr)");
        if(!flash_verify_block(flash_write, firmware_buffer, firmware_buffer_ptr))
        {
          ERROR("verify fail at %x (%u)", flash_write, firmware_buffer_ptr);
          initGlobals();
          return FIRMWARE_UPDATE_ERR_FLASH_FAILED;
        }
        DEBUG(" (ver)\n");
      } else
      {
        DEBUG(" (OK)\n");
      }
      if(file_eof)
      {
        /* firmware update finished */
        DEBUG("fw update done\n");
        initGlobals();
        return FIRMWARE_UPDATE_DONE_SUCCESS;
      }
      firmware_write_offset += firmware_buffer_ptr;
      firmware_buffer_ptr = 0;
    } else
    {
      /* request more data to fill page buffer. An UAVCAN File read returns up to 256 bytes
       * (Only less when EOF is reached) */
      requestFileRead(ins, file_request_ptr);
      DEBUG("req firmware %x\n", file_request_ptr);
      file_request_ptr += 256;
      file_request_pending = 1;
    }
  }
  return FIRMWARE_UPDATE_IN_PROGRESS;
  /* TODO: Timeout/retry for pending requests */
}

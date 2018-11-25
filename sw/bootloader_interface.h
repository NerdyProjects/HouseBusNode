/*
 * bootloader_interface.h
 *
 *  Created on: 24.01.2018
 *      Author: matthias
 */

#ifndef BOOTLOADER_INTERFACE_H_
#define BOOTLOADER_INTERFACE_H_

#include <stdint.h>

#define BOOTLOADER_INTERFACE_VALID_MAGIC 0x6832F3E0

struct bootloader_interface {
  uint32_t magic;
  uint8_t request_from_node_id;
  uint8_t request_file_name[200];
  uint8_t request_file_name_length;
  uint8_t node_id;
};

extern volatile struct bootloader_interface __attribute__ ((section (".bootloader_interface"))) bootloader_interface;

#endif /* BOOTLOADER_INTERFACE_H_ */

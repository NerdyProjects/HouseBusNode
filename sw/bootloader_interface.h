/*
 * bootloader_interface.h
 *
 *  Created on: 24.01.2018
 *      Author: matthias
 */

#ifndef BOOTLOADER_INTERFACE_H_
#define BOOTLOADER_INTERFACE_H_

#include <stdint.h>

struct bootloader_interface {
  uint8_t request_from_node_id;
  uint8_t request_file_name[200];
  uint8_t request_file_name_length;
};

extern struct bootloader_interface bootloader_interface;
#endif /* BOOTLOADER_INTERFACE_H_ */

/*
 * bootloader_interface.c
 *
 *  Created on: 24.01.2018
 *      Author: matthias
 */


#include "bootloader_interface.h"

struct bootloader_interface bootloader_interface __attribute__ ((section (".bootloader_interface"))) __attribute__((used));

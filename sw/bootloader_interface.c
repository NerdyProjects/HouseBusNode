/*
 * bootloader_interface.c
 *
 *  Created on: 24.01.2018
 *      Author: matthias
 */


#include "bootloader_interface.h"

volatile struct bootloader_interface __attribute__ ((used)) __attribute__ ((section (".bootloader_interface"))) bootloader_interface;

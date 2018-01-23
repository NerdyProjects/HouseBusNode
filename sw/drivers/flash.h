/*
 * flash.h
 *
 *  Created on: 23.01.2018
 *      Author: matthias
 */

#ifndef DRIVERS_FLASH_H_
#define DRIVERS_FLASH_H_

int flash_verify_block(void *adr, uint8_t *data, int len)
void flash_write_block(void *adr, uint8_t *data, int len)
void flash_erase_page(void *adr)

#endif /* DRIVERS_FLASH_H_ */

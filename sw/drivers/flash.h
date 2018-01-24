/*
 * flash.h
 *
 *  Created on: 23.01.2018
 *      Author: matthias
 */

#ifndef DRIVERS_FLASH_H_
#define DRIVERS_FLASH_H_

int flash_verify_block(void *adr, uint8_t *data, uint16_t len);
void flash_write_block(void *adr, uint8_t *data, uint16_t len);
void flash_erase_page(void *adr);

#define FLASH_TOT_SIZE 0x10000UL  // 64k.
#define FLASH_PAGE_SIZE 2048UL    // 2k
#define FLASH_PAGE_MASK 0x7FFUL
#define BOOTLOADER_SIZE 16384UL   // 16k

#endif /* DRIVERS_FLASH_H_ */

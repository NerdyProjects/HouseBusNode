/*
 * eeprom24c.h
 *
 *  Created on: 24.01.2018
 *      Author: matthias
 */

#ifndef DRIVERS_EEPROM24C_H_
#define DRIVERS_EEPROM24C_H_

#include "hal.h"

#define EEPROM_PAGE_SIZE 32
int eeprom_init(I2CDriver *i2c);
int eeprom_read(uint16_t adr, uint8_t *out, uint16_t size);
int eeprom_write(uint16_t adr, uint8_t *in, uint16_t size);
#endif /* DRIVERS_EEPROM24C_H_ */

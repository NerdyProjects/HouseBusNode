/*
 * eeprom24c.c
 *
 *  Created on: 24.01.2018
 *      Author: matthias
 */

#include <string.h>
#include "ch.h"
#include "hal.h"
#include "i2c.h"
#include "util.h"
#include "eeprom24c.h"

#define I2C_ADDR 0x50
#define EEPROM_CAPACITY 0x10000
#define EEPROM_PAGE_SIZE 32
#define EEPROM_WRITE_TIMEOUT TIME_MS2I(10)

#define EEPROM_RETRY_COUNT 2

static uint8_t eeprom_address;
static I2CDriver *i2cd;

/* Searches an EEPROM and returns its I2C address (positive) when successful.
 * Returns -1 on error.
 */
int eeprom_init(I2CDriver *i2c)
{
  uint8_t address = I2C_ADDR;
  uint8_t buf;
  i2cd = i2c;
  while(address < I2C_ADDR + 8)
  {
    for(int try = 0; try < EEPROM_RETRY_COUNT; ++try)
    {
      DEBUG("search EE @%x\n", address);
      i2cAcquireBus(i2cd);
      msg_t result = i2cMasterReceiveTimeout(i2cd, address, &buf, 1, TIME_MS2I(5));
      i2cReleaseBus(i2cd);
      if(result == MSG_OK)
      {
        eeprom_address = address;
        DEBUG("found\n");
        return address;
      } else if(result == MSG_TIMEOUT)
      {
        /* unlock driver */
        i2c_init();
      }
    }
    address++;
  }
  return -1;
}

/* Reads a number of bytes from the EEPROM.
 * returns 0 on success or -1 on error.
 */
int eeprom_read(uint16_t adr, uint8_t *out, uint16_t size)
{
  msg_t result;
  if(adr > EEPROM_CAPACITY || adr + size > EEPROM_CAPACITY)
  {
    return -1;
  }
  for(int try = 0; try < EEPROM_RETRY_COUNT; ++try)
  {
    uint8_t buffer[2];
    buffer[0] = adr >> 8;
    buffer[1] = adr & 0x0FF;
    i2cAcquireBus(i2cd);
    result = i2cMasterTransmitTimeout(i2cd, eeprom_address, buffer, 2, out, size, TIME_MS2I(10) + size * TIME_US2I(500));
    i2cReleaseBus(i2cd);
    if(result == MSG_TIMEOUT)
    {
      /* unlock driver */
      i2c_init();
    }
    if(result == MSG_OK)
    {
      return 0;
    }
  }
  return -1;
}

/* waits a little bit and starts "ACK POLLING" afterwards */
static int eeprom_write_ack_poll(void)
{
  int tries = 5;
  /* 10ms should be sufficient for a page write - we do not want to run into timeouts
   * and have enough time, so wait this time before trying something
   */
  chThdSleep(TIME_MS2I(10));
  while(tries--)
  {
    uint8_t c;
    if(eeprom_read(0, &c, 1) == 0)
    {
      return 0;
    }
  }
  return 1;
}

/* Writes a number of bytes to the EEPROM.
 * returns 0 on success or -1 on error.
 */
int eeprom_write(uint16_t adr, uint8_t *in, uint16_t size)
{
  msg_t result;
  uint16_t next_page_adr;
  uint8_t page_space;
  uint8_t write_size = size;
  uint8_t buffer[EEPROM_PAGE_SIZE + 2];
  if((adr > EEPROM_CAPACITY) || (adr + size > EEPROM_CAPACITY))
  {
    return -1;
  }

  /* fill up partial page */
  next_page_adr = (adr & ~(EEPROM_PAGE_SIZE-1)) + EEPROM_PAGE_SIZE;
  page_space = next_page_adr - adr;
  write_size = (page_space < size) ? page_space : size;


  while(write_size)
  {
    uint8_t success = 0;
    buffer[0] = adr >> 8;
    buffer[1] = adr & 0x0FF;
    memcpy(buffer+2, in, write_size);
    for(int try = 0; try < EEPROM_RETRY_COUNT; ++try)
    {
      i2cAcquireBus(i2cd);
      result = i2cMasterTransmitTimeout(i2cd, eeprom_address, buffer, write_size + 2, NULL, 0, TIME_MS2I(20));
      i2cReleaseBus(i2cd);
      if(result == MSG_TIMEOUT)
      {
        /* unlock driver */
        i2c_init();
      }
      if(result == MSG_OK && eeprom_write_ack_poll() == 0)
      {
        success = 1;
        break;
      }
    }
    if(!success)
    {
      return -1;
    }
    size -= write_size;
    adr += write_size;
    in += write_size;
    /* write full pages or a partial last page respectively */
    write_size = (size > EEPROM_PAGE_SIZE) ? EEPROM_PAGE_SIZE : size;
  }

  return 0;
}





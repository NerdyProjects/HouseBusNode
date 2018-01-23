#include "hal.h"

#define FLASH_TOT_SIZE 0x10000UL  // 64k.
#define FLASH_PAGE_SIZE 2048UL    // 2k
#define FLASH_PAGE_MASK 0x7FFUL
#define BOOTLOADER_SIZE 16384UL   // 16k

#define flash_wait_nb() while ( FLASH->SR & FLASH_SR_BSY)

static void unlock(void)
{
  if (FLASH->CR & FLASH_CR_LOCK) {
    //myprintf (CONSOLE, " unlocking ");
    FLASH->KEYR = 0x45670123;
    FLASH->KEYR = 0xcdef89ab;
  }
}

void flash_erase_page(void *adr)
{
  uint16_t i = 0;
  uint32_t *p = (uint32_t *)((uint32_t)adr & ~FLASH_PAGE_MASK);
  for (i = 0; i < FLASH_PAGE_SIZE; ++i)
  {
    if(p[i] != 0xFFFFFFFF)
    {
      break;
    }
  }
  if(i == FLASH_PAGE_SIZE) {
    /* already empty - no erase necessary */
    return;
  }

  unlock();
  flash_wait_nb();
  FLASH->CR |= FLASH_CR_PER;
  FLASH->AR = (uint32_t)adr;
  FLASH->CR |= FLASH_CR_STRT;
  flash_wait_nb();
  FLASH->CR &= ~FLASH_CR_PER;
  FLASH->CR |= FLASH_CR_LOCK;
}

/* compare flash content with given array.
 * returns 1 on match, 0 on dismatch.
 * adr and data need to be 16-bit aligned,
 * len should be an even number of bytes!
 */
int flash_verify_block(void *adr, uint8_t *data, uint16_t len)
{
  uint16_t  i = 0;
  uint16_t *flash = (uint16_t *)adr;
  uint16_t *v = (uint16_t *)data;
  len = (len + 1) / 2;
  for (i = 0; i < len; ++i)
  {
    if (flash[i] != v[i])
    {
      return 0;
    }
  }
  return 1;
}

/* writes len bytes of data to adr.
 * data and addr need to be half-word aligned!
 * len should be an even number of bytes!
 */
void flash_write_block(void *adr, uint8_t *data, uint16_t len)
{
  uint16_t i = 0;
  uint16_t *flash = (uint16_t *)adr;
  uint16_t *v = (uint16_t *)data;
  len = (len + 1) / 2;
  unlock();
  flash_wait_nb();
  FLASH->CR |= FLASH_CR_PG;
  for (i = 0; i < len; ++i)
  {
    if (flash[i] != v[i])
    {
      flash[i] = v[i];
      flash_wait_nb();
    }
  }
  FLASH->CR &= ~FLASH_CR_PG;
  FLASH->CR |= FLASH_CR_LOCK;
}

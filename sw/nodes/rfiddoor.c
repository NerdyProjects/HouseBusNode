#include "node.h"
#include "modules/bme280.h"
#include "drivers/mfrc522.h"

void app_init(void)
{
  bme280_app_init();
  TM_MFRC522_Init();
}

void pwdAuth(uint32_t pwd) {
  TM_MFRC522_Status_t status;
  uint8_t buf[64];
  uint16_t rxLen;
  buf[0] = 0x1B;
  buf[1] = pwd;
  buf[2] = pwd >> 8;
  buf[3] = pwd >> 16;
  buf[4] = pwd >> 24;
	TM_MFRC522_CalculateCRC(buf,5, &buf[5]);
	status = TM_MFRC522_ToCard(PCD_TRANSCEIVE, buf, 7, buf, &rxLen);
  if(rxLen == 4) {
    if((buf[0] & 0x0F) != 0x0A) {
      /* NAK received */
    } else {
      /* ACK received */
    }
  } else if(rxLen == (4*8)) {
    /* PACK + CRC received */
    TM_MFRC522_CalculateCRC(buf, 2, &buf[4]);
  }


}

void card_cycle(void)
{
  uint8_t CardID[5];
  if (TM_MFRC522_Check(CardID) == MI_OK) {
    node_debug(LOG_LEVEL_DEBUG, "MFRC522", "Card detect");
    node_debug_int(LOG_LEVEL_DEBUG, "ID", CardID[0] << 24 | CardID[1] << 16 | CardID[2] << 8 | CardID[3]);
  } else {
    node_debug(LOG_LEVEL_DEBUG, "MFRC522", "No card detected");
  }

}

/** Card layout:
 * 9 byte serial number p0/p1/p2.0; where effectively only 6 bytes matter (p0.1-p1.2)
 * we need a 32 bit password for each card (PWD_AUTH) and a 16 bit PACK (can be substring
 * of password as both are known to the card and password is transmitted over air first anyway?)
 * We need a random password per card, 32 bit should be more than enough, that is stored on the
 * password protected blocks.

*/
void app_tick(void)
{
  bme280_app_read();
  card_cycle();
}

void app_fast_tick(void)
{

}

void app_config_update(void)
{

}

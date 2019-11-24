#include "node.h"
#include "modules/bme280.h"
#include "drivers/mfrc522.h"

void app_init(void)
{
  bme280_app_init();
  TM_MFRC522_Init();
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

#include "modules/brightness_data.h"

#define PORT GPIOA
#define PIN 15

void app_init(void)
{
  palClearPad(PORT, PIN);
  palSetPadMode(PORT, PIN, PAL_MODE_OUTPUT_PUSHPULL);
  bme280_app_init();
}

void app_config_update(void)
{
}

void app_fast_tick(void)
{
}

void app_tick(void)
{
  bme280_app_read();
  if (brightness > 2) {
    palClearPad(PORT, PIN);
  }
  else {
    palSetPad(PORT, PIN);
  }
}

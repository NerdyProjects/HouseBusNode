#include "hal.h"
#include "node.h"
#include "config.h"
#include "../drivers/tone.h"
#include "modules/bme280.h"
#include "modules/melodies.h"

void app_init(void)
{
  tone_init();
  bme280_app_init();
}

void app_fast_tick(void)
{
}

void app_tick(void)
{
  bme280_app_read();
}

#include "hal.h"
#include "node.h"
#include "config.h"
#include "../drivers/tone.h"
#include "modules/bme280.h"
#include "modules/melodies.h"
#include "modules/dewpoint.h"
#include "modules/temperature_receiver.h"

static uint16_t wallTemperatureFactor;
static sysinterval_t firstAlarmAfter;
static sysinterval_t repeatAlarmEvery;

static void humidity_alarm_tick(void)
{
  int16_t wallTemperatureDifference;
  static systime_t lastAlarmAt;
  static uint8_t lastAlarmType;
  if(approximateWallTemperatureDifferenceToDewPoint(&wallTemperatureDifference, wallTemperatureFactor) == 0)
  {
    if(wallTemperatureDifference > 0)
    {
      lastAlarmType = 0;
    } else
    {
      if(lastAlarmType == 0)
      {
        lastAlarmType = 1;
        lastAlarmAt = chVTGetSystemTimeX();
      }
      if(lastAlarmType == 1 && chVTTimeElapsedSinceX(lastAlarmAt) >= firstAlarmAfter)
      {
        lastAlarmType = 2;
        lastAlarmAt = chVTGetSystemTimeX();
        melody_play(config_get_uint(CONFIG_HUMIDITY_ALARM_FIRST_MELODY), config_get_uint(CONFIG_HUMIDITY_ALARM_VOLUME));
      }
      if(lastAlarmType == 2 && chVTTimeElapsedSinceX(lastAlarmAt) >= repeatAlarmEvery)
      {
        lastAlarmAt = chVTGetSystemTimeX();
        melody_play(config_get_uint(CONFIG_HUMIDITY_ALARM_REPEAT_MELODY), config_get_uint(CONFIG_HUMIDITY_ALARM_VOLUME));
      }
    }
  }
}

static void read_config(void)
{
  uint32_t t = config_get_uint(CONFIG_TEMPERATURE_RECEIVER_TARGET_NODE_ID);
  setReceiverTarget(t);
  wallTemperatureFactor = config_get_uint(CONFIG_WALL_TEMPERATURE_FACTOR_BY_1024);
  firstAlarmAfter = TIME_S2I(config_get_uint(CONFIG_HUMIDITY_ALARM_FIRST_AFTER_S));
  repeatAlarmEvery = TIME_S2I(config_get_uint(CONFIG_HUMIDITY_ALARM_REPEAT_INTERVAL_S));
}

void app_config_update(void)
{
  read_config();
}

void app_init(void)
{
  tone_init();
  bme280_app_init();
  read_config();
}

void app_fast_tick(void)
{
  melody_tick();
}

void app_tick(void)
{
  bme280_app_read();
  humidity_alarm_tick();
}

/*
 * bme280_node.c
 *
 *  Created on: 21.01.2018
 *      Author: matthias
 */

#include <drivers/bme280_ll.h>
#include <string.h>
#include "ch.h"
#include "hal.h"
#include "node.h"
#include "util.h"
#include "drivers/i2c.h"

#define BME280_TIMEOUT TIME_MS2I(50)

static struct bme280_dev bme[2];
uint8_t bme_presence = 0;         /* LSB marks presence of primary/secondary BME I2C, 1 << 0, 1 << 1 */

int16_t BMECentiTemperature[2];   /* -327(.)68 - 327(.)68 degrees */
uint32_t BMEMilliHumidity[2];     /* 0 - 100(.)000 % */
uint32_t BMECentiPressure[2];     /* 300(.)00-1100(.)00 mBar  */

static void delay_ms(uint32_t period) {
  /* quirks: Library uses 1 and 2ms delays that are not really necessary to be exact */
  chThdSleep(TIME_MS2I(2));
}

static int8_t i2c_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len) {
  i2cAcquireBus(&I2CD1);
  msg_t res = i2cMasterTransmitTimeout(&I2CD1, dev_id, &reg_addr, 1, data, len, BME280_TIMEOUT);
  i2cReleaseBus(&I2CD1);
  if(res == MSG_TIMEOUT)
  {
    /* unlock driver */
    i2c_init();
  }
  if(res == MSG_OK)
  {
    return BME280_OK;
  } else
  {
    return BME280_E_COMM_FAIL;
  }
}

static int8_t i2c_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len) {
  /* the definition of this write function inside the BME code is actually just stupid...
   * so we have to order in the reg_addr which needs to be send in front of data.
   * Let's assume the same 20 bytes limit as the lib assumes.
   */
  uint8_t temp[20];
  temp[0] = reg_addr;
  msg_t res;
  if(len > 19) {
    return BME280_E_INVALID_LEN;
  }
  for(uint8_t i = 0; i < len; ++i) {
    temp[i+1] = data[i];
  }
  i2cAcquireBus(&I2CD1);
  res = i2cMasterTransmitTimeout(&I2CD1, dev_id, temp, len+1, NULL, 0, BME280_TIMEOUT);
  i2cReleaseBus(&I2CD1);
  if(res == MSG_TIMEOUT)
  {
    /* unlock driver */
    i2c_init();
  }
  if(res == MSG_OK)
  {
    return BME280_OK;
  } else
  {
    return BME280_E_COMM_FAIL;
  }
}

/* Valid flags: 1 - humidity valid, 2 - pressure valid */
static void bme280_broadcast(int32_t centiCelsiusTemperature, uint32_t milliRelativeHumidity, uint32_t centiBarPressure, uint8_t validFlags)
{
  uint8_t buffer[HOMEAUTOMATION_ENVIRONMENT_MESSAGE_SIZE];
  static uint8_t transfer_id = 0;

  /* data contains:
   *  humidity in millipercent (percent = hum / 1000) [0..100000] -> 17 bit
   *  pressure in 10^-2 mbar (mbar = pres / 100) [30000-110000] -> 18 bit
   *  temperature in centidegrees (degree = temp / 100) [-4000..8500] -> 19 bit
   *  ToDo: Temperature only needs 15 bits
   */
  memset(buffer, 0, HOMEAUTOMATION_ENVIRONMENT_MESSAGE_SIZE);
  canardEncodeScalar(buffer, 0, 2, &validFlags);
  canardEncodeScalar(buffer, 2, 19, &centiCelsiusTemperature);
  canardEncodeScalar(buffer, 21, 17, &milliRelativeHumidity);
  canardEncodeScalar(buffer, 38, 18, &centiBarPressure);
  canardLockBroadcast(&canard,
        HOMEAUTOMATION_ENVIRONMENT_DATA_TYPE_SIGNATURE,
        HOMEAUTOMATION_ENVIRONMENT_DATA_TYPE_ID, &transfer_id,
        CANARD_TRANSFER_PRIORITY_LOW, buffer, HOMEAUTOMATION_ENVIRONMENT_MESSAGE_SIZE);
}

void bme280_app_read(void)
{
  struct bme280_data data;
  for(int i = 0; i < 2; ++i)
  {
    bme280_get_sensor_data(BME280_ALL, &data, &bme[0]);
    BMECentiTemperature[i] = data.temperature;
    BMEMilliHumidity[i] = data.humidity;
    BMECentiPressure[i] = data.pressure;
  }

  /* until the datatype has been refactored to allow multiple sensors, we just send the first one */
  if(bme_presence & (1 << 0))
  {
    bme280_broadcast(BMECentiTemperature[0], BMEMilliHumidity[0], BMECentiPressure[0], 3);
  } else if(bme_presence & (1 << 1))
  {
    bme280_broadcast(BMECentiTemperature[1], BMEMilliHumidity[1], BMECentiPressure[1], 3);
  }

}

void bme280_app_init(void) {
  int8_t res;
  bme_presence = 0;
  for(int i = 0; i < 2; ++i)
  {
    bme[i].intf = BME280_I2C_INTF;
    bme[i].dev_id = (i ? BME280_I2C_ADDR_SEC : BME280_I2C_ADDR_PRIM);
    bme[i].read = i2c_read;
    bme[i].write = i2c_write;
    bme[i].delay_ms = delay_ms;
    res = bme280_init(&bme[i]);
    if (res == BME280_OK)
    {
      bme_presence |= (1 << i);
    }
    /* Pressure/temperature gets FIR filtered, ~5.5 seconds step response time. */
    bme[i].settings.osr_h = BME280_OVERSAMPLING_8X;
    bme[i].settings.osr_p = BME280_OVERSAMPLING_2X;
    bme[i].settings.osr_t = BME280_OVERSAMPLING_2X;
    bme[i].settings.filter = BME280_FILTER_COEFF_4;
    bme[i].settings.standby_time = BME280_STANDBY_TIME_500_MS;
    bme280_set_sensor_settings(BME280_FILTER_SEL | BME280_STANDBY_SEL | BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL, &bme[i]);
    bme280_set_sensor_mode(BME280_NORMAL_MODE, &bme[i]);
  }
}

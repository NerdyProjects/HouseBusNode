/*
 * bme280_node.c
 *
 *  Created on: 21.01.2018
 *      Author: matthias
 */

#include "ch.h"
#include "hal.h"
#include "drivers/bme280.h"
#include "node.h"
#include "uavcan.h"
#include "util.h"
#include <string.h>

#define BME280_TIMEOUT MS2ST(50)

static struct bme280_dev bme;

void delay_ms(uint32_t period) {
  chThdSleep(MS2ST(period));

}

int8_t i2c_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len) {
  return i2cMasterTransmitTimeout(&I2CD1, dev_id, &reg_addr, 1, data, len, BME280_TIMEOUT);
}

int8_t i2c_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len) {
  /* the definition of this write function inside the BME code is actually just stupid...
   * so we have to order in the reg_addr which needs to be send in front of data.
   * Let's assume the same 20 bytes limit as the lib assumes.
   */
  uint8_t temp[20];
  temp[0] = reg_addr;
  if(len > 19) {
    return BME280_E_INVALID_LEN;
  }
  for(uint8_t i = 0; i < len; ++i) {
    temp[i+1] = data[i];
  }
  return i2cMasterTransmitTimeout(&I2CD1, dev_id, temp, len+1, NULL, 0, BME280_TIMEOUT);
}

static THD_WORKING_AREA(waBME280Thread, 256);
static THD_FUNCTION(BME280Thread, arg)
{
  struct bme280_data data;
  uint8_t buffer[HOMEAUTOMATION_ENVIRONMENT_MESSAGE_SIZE];
  uint8_t transfer_id = 0;
  (void) arg;
  chRegSetThreadName("BME280");
  while(1) {
    chThdSleep(S2ST(2));
    bme280_get_sensor_data(BME280_ALL, &data, &bme);
    /* data contains:
     *  humidity in millipercent (percent = hum / 1000) [0..100000] -> 17 bit
     *  pressure in 10^-2 mbar (mbar = pres / 100) [30000-110000] -> 18 bit
     *  temperature in centidegrees (degree = temp / 100) [-4000..85000] -> 18 bit
     */
    memset(buffer, 0, HOMEAUTOMATION_ENVIRONMENT_MESSAGE_SIZE);
    canardEncodeScalar(buffer, 0, 18, &data.temperature);
    canardEncodeScalar(buffer, 18, 17, &data.humidity);
    canardEncodeScalar(buffer, 35, 18, &data.pressure);
    const int bc_res = canardBroadcast(&canard,
          HOMEAUTOMATION_ENVIRONMENT_DATA_TYPE_SIGNATURE,
          HOMEAUTOMATION_ENVIRONMENT_DATA_TYPE_ID, &transfer_id,
          CANARD_TRANSFER_PRIORITY_LOW, buffer, HOMEAUTOMATION_ENVIRONMENT_MESSAGE_SIZE);
    if (bc_res <= 0)
    {
      ERROR("Could not broadcast node status; error %d\n", bc_res);
    }
    node_tx_request();
  }
}

void bme280_node_init(void) {
  int8_t res;
  bme.intf = BME280_I2C_INTF;
  bme.dev_id = BME280_I2C_ADDR_PRIM;
  bme.read = i2c_read;
  bme.write = i2c_write;
  bme.delay_ms = delay_ms;
  res = bme280_init(&bme);
  if(res == BME280_E_DEV_NOT_FOUND) {
    bme.dev_id = BME280_I2C_ADDR_SEC;
    res = bme280_init(&bme);
    if(res != BME280_OK) {
      return;
    }
  }
  /* Pressure/temperature gets FIR filtered, ~5.5 seconds step response time. */
  bme.settings.osr_h = BME280_OVERSAMPLING_8X;
  bme.settings.osr_p = BME280_OVERSAMPLING_2X;
  bme.settings.osr_t = BME280_OVERSAMPLING_2X;
  bme.settings.filter = BME280_FILTER_COEFF_4;
  bme.settings.standby_time = BME280_STANDBY_TIME_500_MS;
  bme280_set_sensor_settings(BME280_FILTER_SEL | BME280_STANDBY_SEL | BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL, &bme);
  bme280_set_sensor_mode(BME280_NORMAL_MODE, &bme);
  chThdCreateStatic(waBME280Thread, sizeof(waBME280Thread), LOWPRIO, BME280Thread,
        NULL);
  return;
}

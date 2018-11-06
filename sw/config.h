/*
 * config.h
 *
 *  Created on: 24.01.2018
 *      Author: matthias
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdint.h>
#include "hal.h"

#define CONF_PLAIN(x, v, t) x
#define CONF_STRINGIFY(x, v, t) #x
#define CONF_LENGTH(x, v, t) v
#define CONF_TYPE(x, v, t) t

/* mismatch of MAGIC should invalidate config */
#define CONFIGURATION_MAGIC 0xEF7389AC
/* meaning of version differences should be handled by the user */
#define CONFIGURATION_VERSION 0x00000001
#define CONFIG_MAX_SIZE 16384

enum config_param_type {
  CONFIG_PARAM_VOID,
  CONFIG_PARAM_INT,
  CONFIG_PARAM_STRING
};

typedef enum config_param_type config_param_type_t;

/* These can be removed, but it probably needs invalidating the config:
  CONFIG_HAS_WATER_REFILL_OUTPUT_PIN
  CONFIG_PUMP_TRIGGER_SOURCE
  CONFIG_PUMP_STOP_CONDITION_1
  CONFIG_PUMP_STOP_CONDITION_2
*/
#define CONFIG(X) \
  X(CONFIG_MAGIC, 4, CONFIG_PARAM_INT), \
  X(CONFIG_VERSION, 4, CONFIG_PARAM_INT), \
  X(CONFIG_NODE_ID, 4, CONFIG_PARAM_INT), \
  X(CONFIG_NODE_NAME, 80, CONFIG_PARAM_STRING), \
  X(CONFIG_HAS_ANALOG_CONDUCTION_SENSOR, 4, CONFIG_PARAM_INT), \
  X(CONFIG_HAS_WATER_REFILL_OUTPUT_PIN, 4, CONFIG_PARAM_INT), \
  X(CONFIG_PUMP_TRIGGER_SOURCE, 4, CONFIG_PARAM_INT), \
  X(CONFIG_PUMP_STOP_CONDITION_1, 4, CONFIG_PARAM_INT), \
  X(CONFIG_PUMP_STOP_CONDITION_2, 4, CONFIG_PARAM_INT), \
  X(CONFIG_EVENTCOUNT0_PORT, 4, CONFIG_PARAM_INT), \
  X(CONFIG_EVENTCOUNT0_PIN, 4, CONFIG_PARAM_INT), \
  X(CONFIG_EVENTCOUNT0_MIN_STABLE_MS, 4, CONFIG_PARAM_INT), \
  X(CONFIG_EVENTCOUNT1_PORT, 4, CONFIG_PARAM_INT), \
  X(CONFIG_EVENTCOUNT1_PIN, 4, CONFIG_PARAM_INT), \
  X(CONFIG_EVENTCOUNT1_MIN_STABLE_MS, 4, CONFIG_PARAM_INT), \
  X(CONFIG_EVENTCOUNT2_PORT, 4, CONFIG_PARAM_INT), \
  X(CONFIG_EVENTCOUNT2_PIN, 4, CONFIG_PARAM_INT), \
  X(CONFIG_EVENTCOUNT2_MIN_STABLE_MS, 4, CONFIG_PARAM_INT), \
  X(CONFIG_EVENTCOUNT3_PORT, 4, CONFIG_PARAM_INT), \
  X(CONFIG_EVENTCOUNT3_PIN, 4, CONFIG_PARAM_INT), \
  X(CONFIG_EVENTCOUNT3_MIN_STABLE_MS, 4, CONFIG_PARAM_INT), \
  X(CONFIG_SML_PORT, 4, CONFIG_PARAM_INT), \
  X(CONFIG_DIMMER_PIN, 4, CONFIG_PARAM_INT), \
  X(CONFIG_DIMMER_DUTY_CYCLE, 4, CONFIG_PARAM_INT), \
  X(CONFIG_DIMMER_PERIOD_IN_HALFWAVES, 4, CONFIG_PARAM_INT), \
  X(CONFIG_PUMP_RECEIVER_PIN, 4, CONFIG_PARAM_INT), \
  X(CONFIG_PUMP_RECEIVER_TARGET_NODE_ID, 4, CONFIG_PARAM_INT), \
  X(CONFIG_HAS_LIGHT_CONTROL, 4, CONFIG_PARAM_INT), \
  X(CONFIG_HEATER_FLOW_MIN_TARGET_TEMP, 4, CONFIG_PARAM_INT), \
  X(CONFIG_HEATER_FLOW_HYSTERESIS_TURN_ON, 4, CONFIG_PARAM_INT), \
  X(CONFIG_HEATER_FLOW_HYSTERESIS_TURN_OFF, 4, CONFIG_PARAM_INT), \
  X(CONFIG_PARAM_LAST, 0, CONFIG_PARAM_VOID)


enum config_param {
  CONFIG(CONF_PLAIN)
};
typedef enum config_param config_param_t;

enum config_status {
  CONFIG_STATUS_OK,
  CONFIG_STATUS_INVALID,
  CONFIG_STATUS_VERSION_MISMATCH
};
typedef enum config_status config_status_t;

#define CONFIG_PARAM_SIZE_INIT CONFIG(CONF_LENGTH)
#define CONFIG_PARAM_NAME_INIT CONFIG(CONF_STRINGIFY)
#define CONFIG_PARAM_TYPE_INIT CONFIG(CONF_TYPE)

uint8_t config_get_param_size(config_param_t param);
int config_get_param_type(config_param_t param);
int config_get(config_param_t param, void *dst, uint8_t *valid);
int config_get_name(config_param_t param, uint8_t *dst);
uint32_t config_get_uint(config_param_t param);
int32_t config_get_int(config_param_t param);
int config_set(config_param_t param, void *src, uint8_t size);
int config_set_uint(config_param_t param, uint32_t v);
int config_get_id_by_name(uint8_t *name, uint8_t name_len);
int config_init(I2CDriver *i2c);

#define CONFIG_OK 0
#define CONFIG_NO_SUCH_PARAMETER -1
#define CONFIG_MEMORY_ERROR -2


#endif /* CONFIG_H_ */

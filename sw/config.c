#include <string.h>
#include "config.h"
#include "drivers/eeprom24c.h"
#include "util.h"

static uint8_t config_param_size[] = { CONFIG_PARAM_SIZE_INIT };
static const char *config_param_name[] = { CONFIG_PARAM_NAME_INIT };
static config_param_type_t config_param_type[] = { CONFIG_PARAM_TYPE_INIT };
static config_status_t status = CONFIG_STATUS_INVALID;

/* retrieves the used data field length for a parameter.
 */
uint8_t config_get_param_size(config_param_t param)
{
  if(param >= CONFIG_PARAM_LAST)
  {
    return 0;
  }
  return config_param_size[param];
}

int config_get_param_type(config_param_t param)
{
  if(param >= CONFIG_PARAM_LAST)
  {
    return -1;
  }
  return config_param_type[param];
}

static uint8_t get_param_storage_size(config_param_t param)
{
  /* Every parameter is prefixed with its valid length */
  return config_get_param_size(param) + 1;
}


/* gets storage byte offset for given configuration parameter */
static uint32_t get_param_offset(config_param_t param) {
  uint32_t offset = 0;
  while(param--) {
    offset += get_param_storage_size(param);
  }

  return offset;
}

int config_get_name(config_param_t param, uint8_t *dst)
{
  if(param >= CONFIG_PARAM_LAST)
  {
    return CONFIG_NO_SUCH_PARAMETER;
  }
  strcpy(dst, config_param_name[param]);
  return CONFIG_OK;
}

/* searches for a parameter with the given name and returns the parameter ID.
 * Although names are internally null-terminated, this interface is optimised for
 * UAVCAN length-prefixed strings.
 */
int config_get_id_by_name(uint8_t *name, uint8_t name_len)
{
  int res;
  for(res = 0; res < CONFIG_PARAM_LAST; res++)
  {
    const char *param_name = config_param_name[res];
    if((strlen(param_name) == name_len) && (memcmp(name, param_name, name_len) == 0))
    {
      return res;
    }
  }
  return CONFIG_NO_SUCH_PARAMETER;
}

/* returns a configuration parameter.
 * The caller needs to supply a piece of memory where it can be written to.
 * The caller needs to find out the size and type of that configuration parameter.
 * If the pointer valid is given, the valid length (e.g. the size written to it)
 * is given there.
 * Only the valid part of the data can be used, e.g. the part that has been written
 * before. The output memory does only need to provide space for the valid length.
 * To retrieve only the valid length, the dst pointer can be null.
 */
int config_get(config_param_t param, void *dst, uint8_t *valid) {
  if(param >= CONFIG_PARAM_LAST)
  {
    if(valid) {
      *valid = 0;
    }
    return CONFIG_NO_SUCH_PARAMETER;
  }
  uint32_t offset = get_param_offset(param);
  uint8_t valid_length;
  int res = eeprom_read(offset, &valid_length, 1);
  if (res < 0) {
    return CONFIG_MEMORY_ERROR;
  }
  if(valid_length > config_get_param_size(param))
  {
    valid_length = config_get_param_size(param);
  }
  if (valid) {
    *valid = valid_length;
  }
  if (dst) {
    res = eeprom_read(offset + 1, (uint8_t *)dst, valid_length);
  }
  if (res < 0)
  {
    return CONFIG_MEMORY_ERROR;
  }
  return CONFIG_OK;
}

/* returns an unsigned integer configuration parameter.
 * convenience method to use the parameter as return value.
 */
uint32_t config_get_uint(config_param_t param) {
  uint32_t res = 0;
  /* only works correct with little endian machines - we just assume that here */
  if(config_get_param_size(param) <= 4)
  {
    config_get(param, &res, NULL);
  }
  return res;
}

/* returns a signed integer configuration parameter.
 * convenience method to use the parameter as return value.
 */
int32_t config_get_int(config_param_t param)
{
  int32_t res = 0;
  if (config_get_param_size(param) <= 4)
  {
    config_get(param, &res, NULL);
  }
  return res;
}

/* writes a configuration parameter.
 * The caller can supply a length up to the reserved space of the parameter field.
 * If given size is bigger than the actual parameter size, excess data is silently ignored.
 */
int config_set(config_param_t param, void *src, uint8_t size)
{
  int res;
  if(param >= CONFIG_PARAM_LAST)
  {
    return CONFIG_NO_SUCH_PARAMETER;
  }
  uint32_t offset = get_param_offset(param);
  uint8_t reserved_size = config_get_param_size(param);
  if(size > reserved_size)
  {
    size = reserved_size;
  }
  res = eeprom_write(offset, &size, 1);
  res |= eeprom_write(offset + 1, src, size);
  if(res == 0)
  {
    return CONFIG_OK;
  } else
  {
    return CONFIG_MEMORY_ERROR;
  }
}

int config_set_uint(config_param_t param, uint32_t v)
{
  return config_set(param, &v, 4);
}

int config_init(I2CDriver *i2c)
{
  uint32_t magic = 0;
  uint32_t version = 0;
  if(eeprom_init(i2c) < 0)
  {
    return -1;
  }
  DEBUG("read magic ");
  config_get(CONFIG_MAGIC, &magic, 0);
  DEBUG("%x", magic);
  config_get(CONFIG_VERSION, &version, 0);
  DEBUG(": %u\n", version);
  if(magic == CONFIGURATION_MAGIC)
  {
    DEBUG("Configuration accepted\n");
    if(version == CONFIGURATION_VERSION)
    {
      status = CONFIG_STATUS_OK;
    } else
    {
      status = CONFIG_STATUS_VERSION_MISMATCH;
    }
  } else
  {
    status = CONFIG_STATUS_INVALID;
    /* clear configuration storage */
    DEBUG("Clearing configuration...\n");
    wdgReset(&WDGD1);
    for(int i = 0; i < CONFIG_MAX_SIZE/EEPROM_PAGE_SIZE; ++i)
    {
      uint8_t buf[EEPROM_PAGE_SIZE];
      memset(buf, 0, EEPROM_PAGE_SIZE);
      eeprom_write(EEPROM_PAGE_SIZE*i, buf, EEPROM_PAGE_SIZE);
    }
    /* Initialize sane defaults for UAVCAN configuration */
    DEBUG("Loading default node configuration...\n");
    wdgReset(&WDGD1);
    config_set_uint(CONFIG_NODE_ID, 127);
    config_set(CONFIG_NODE_NAME, "Unconfigured node", 17);
    config_set_uint(CONFIG_MAGIC, CONFIGURATION_MAGIC);
    config_set_uint(CONFIG_VERSION, CONFIGURATION_VERSION);
  }
  return 0;
}

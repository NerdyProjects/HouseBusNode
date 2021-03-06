#include "hal.h"
#include "../config.h"
#include "../node.h"
#include "string.h"

SerialDriver *port = NULL;

const uint8_t prefix[] = {0x77, 0x07, 0x01, 0x00};
//, 0x01, 0x08, 0x00, 0xff, 0x65, 0x00, 0x01, 0x01, 0x82, 0x01, 0x62, 0x1e, 0x52, 0xff, 0x59};
typedef enum
{
  ST_SEARCH_PREFIX,
  ST_PARSE_OBIS,
  ST_SKIP_STATUS,
  ST_SKIP_VALTIME,
  ST_UNIT,
  ST_SKIP_SCALER,
  ST_VALUE
} SML_STATES;

static SerialDriver* port_to_serial(uint8_t port)
{
  switch(port)
  {
  case 1:
    return &SD1;
    break;
  default:
    return 0;
    break;
  }
}

void app_init(void)
{
  port = port_to_serial(config_get_uint(CONFIG_SML_PORT));
}

uint8_t sml_is_present(void)
{
  return (port != 0);
}

static void broadcast_meter(uint8_t* obis, uint8_t unit, uint8_t* value)
{
  static uint8_t transfer_id = 0;
  uint8_t buffer[12];
  buffer[0] = obis[0];
  buffer[1] = obis[1];
  buffer[2] = obis[2];
  buffer[3] = unit;
  memcpy(&buffer[4], value, 8);

  canardLockBroadcast(&canard,
          HOMEAUTOMATION_OBIS_DATA_TYPE_SIGNATURE,
          HOMEAUTOMATION_OBIS_DATA_TYPE_ID, &transfer_id,
          CANARD_TRANSFER_PRIORITY_LOW, buffer, sizeof(buffer));
}

/**
 * processes incoming SML data.
 * Generates an obis message for 1.8.0 and 2.8.0 codes.
 */
void app_fast_tick(void)
{
  static uint8_t state = ST_SEARCH_PREFIX;
  static uint8_t substate = 0;
  static uint8_t value[8];
  static uint8_t obis[3];
  static uint8_t state_buf;
  static uint8_t sign;
  static uint8_t unit;

  msg_t msg;

  if(!sml_is_present())
  {
    return;
  }

  while((msg = sdGetTimeout(port, TIME_IMMEDIATE)) != MSG_TIMEOUT)
  {
    switch(state)
    {
    case ST_SEARCH_PREFIX:
      if(msg == prefix[substate])
      {
        substate++;
      } else
      {
        substate = 0;
      }
      if(substate == sizeof(prefix))
      {
        state = ST_PARSE_OBIS;
        substate = 0;
      }
      break;
    case ST_PARSE_OBIS:
      obis[substate++] = msg;
      if(substate == 4)
      {
        state = ST_SKIP_STATUS;
        substate = 0;
      }
      break;
    case ST_SKIP_STATUS:
    case ST_SKIP_VALTIME:
    case ST_SKIP_SCALER:
      if(substate == 0)
      {
        state_buf = msg & 0x0F;
      }
      substate++;
      if(substate >= state_buf)
      {
        state++;
        substate = 0;
      }
      break;
    case ST_UNIT:
      if(substate == 0)
      {
        state_buf = msg & 0x0F;
      }
      substate++;
      if(substate >= state_buf)
      {
        unit = msg;
        state++;
        substate = 0;
      }
      break;
    case ST_VALUE:
      if(substate == 0)
      {
        state_buf = msg & 0x0F; /* SML value length including TL byte */
        sign = ((msg & 0xF0) == 0x50); /* data type is signed */
        if((((msg & 0xF0) != 0x60) && ((msg & 0xF0) != 0x50)) || state_buf > 9)
        {
          /* neither INT nor UINT -> discard */
          /* longer than 64 it -> discard */
          state = ST_SEARCH_PREFIX;
          substate = 0;
        }
      }
      else
      {
        /* Proper placement in 64 bit LE int */
        /* SML sends 8/16/32/64 it big endian int.
         * 8: write to value [0]
         * 16: write to value [1], then [0]
         * 64: write to value [7], ..., then [0]
         * Sign extension happens later, except for 64 bit types.
         * state_buf: byte width of SML type (+ 1), so 2/3/5/9
         */
        value[state_buf - 1 - substate] = msg;
      }
      substate++;
      if(substate >= state_buf)
      {
        uint8_t ext = ((value[state_buf - 2] & 0x80) && sign) ? 0xFF : 0x00;
        while(state_buf < 9)
        {
          /* state_buf is 9 for 64bit, otherwise we need to to proper sign extend/zero fill now.
           * ex.: 5 for 32 bit -> need to fill 4,5,6,7
           */
          value[state_buf - 1] = ext;
          state_buf++;
        }
        broadcast_meter(obis, unit, value);
        state = ST_SEARCH_PREFIX;
        substate = 0;
      }
      break;
    default:
      node_debug(LOG_LEVEL_ERROR, "SML", "Switch default");
      break;
    }
  }
}

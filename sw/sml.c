/*
 * sml.c
 *
 *  Created on: 02.04.2018
 *      Author: matthias
 */

#include "hal.h"
#include "config.h"

SerialDriver *port = NULL;

const uint8_t prefix[] = {0x77, 0x07, 0x01, 0x00, 0x01, 0x08, 0x00, 0xff, 0x65, 0x00, 0x01, 0x01, 0x82, 0x01, 0x62, 0x1e, 0x52, 0xff, 0x59};

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

void sml_init(void)
{
  port = port_to_serial(config_get_uint(CONFIG_SML_PORT));
}

uint8_t sml_is_present(void)
{
  return (port != 0);
}

/**
 * processes incoming SML data.
 * returns a pointer to a valid meter reading or NULL otherwise.
 * The pointer is some internal memory, valid until the next call of tick.
 */
uint8_t *sml_tick(void)
{
  static uint8_t message_state = 0;
  static uint8_t buffer_idx = 7;
  static uint8_t buffer[8];

  msg_t msg;
  while((msg = sdGetTimeout(port, TIME_IMMEDIATE)) != MSG_TIMEOUT)
  {
    if(message_state == sizeof(prefix))
    {
      buffer[buffer_idx] = msg;
      buffer_idx--;
      if(buffer_idx > 7)
      {
        buffer_idx = 7;
        message_state = 0;
        return buffer;
      }
    } else if(msg == prefix[message_state])
    {
      message_state++;
    } else
    {
      message_state = 0;
    }
  }
  return NULL;
}


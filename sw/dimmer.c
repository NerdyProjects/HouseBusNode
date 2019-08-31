/*
 * dimmer.c
 */

#include <hal.h>
#include <ch.h>
#include "node.h"
#include "drivers/analog.h"
#include "config.h"
#include "conduction_sensor.h"

#define PORT GPIOB
#define TICK_MS 5
#define halfwaves_to_tick(h) ((h)*10/TICK_MS)

static uint8_t pin;
static volatile uint8_t halfwaves_on;
static volatile uint8_t halfwaves_total;

uint8_t dimmer_is_present(void)
{
  return pin < 16;
}

void dimmer_init(void)
{
  pin = config_get_uint(CONFIG_DIMMER_PIN);
  /* Pins numbered 1..16 */
  if(!pin)
  {
    pin = 16;
    return;
  }
  pin--;

  palClearPad(PORT, pin);
  palSetPadMode(PORT, pin, PAL_MODE_OUTPUT_PUSHPULL);

  palSetPadMode(GPIOA, 7, PAL_MODE_INPUT_PULLUP);
}

void dimmer_read_config(void)
{
  halfwaves_total = config_get_uint(CONFIG_DIMMER_PERIOD_IN_HALFWAVES);
  uint8_t dc = config_get_uint(CONFIG_DIMMER_DUTY_CYCLE);
  halfwaves_on = (dc * halfwaves_total) / 100;
}

void dimmer_set_dc(uint8_t dc)
{
  halfwaves_on = (dc * halfwaves_total) / 100;
}

void dimmer_tick(void)
{
  static uint8_t cnt;
  if (cnt < halfwaves_to_tick(halfwaves_on)) {
    palSetPad(PORT, pin);
  } else {
    palClearPad(PORT, pin);
  }

  cnt++;
  if (cnt >= halfwaves_to_tick(halfwaves_total)) {
    cnt = 0;
  }

}

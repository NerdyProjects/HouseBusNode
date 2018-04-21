/*
 * pump_receiver.c
 */

#include "node.h"
#include "config.h"
#include "time_data.h"

#define PORT GPIOB
#define TRANSFER_ERROR_MASK 0x1
#define TRANSFER_TOP_CONDUCTION_SENSOR_MASK 0x128
#define MAX_PUMP_ON_SECONDS 60*5
#define CHECK_PERIOD_SECONDS 60*120

static uint8_t pin;
static uint8_t target_node;
static volatile uint32_t time_since_last_data_seconds;
static volatile uint32_t pump_on_seconds;
static volatile uint32_t time_since_last_check_seconds;
static volatile bool target_has_error;
static volatile bool target_is_full;
static volatile bool is_pump_on;

bool pump_receiver_is_present(void)
{
  if (pin >= 16 || target_node == 0xFF)
  {
    return false;
  }
  return true;
}

static bool pump_receiver_is_up_to_date(void)
{
  return time_since_last_data_seconds < 3;
}

static void turn_pump_on(void)
{
  palSetPad(PORT, pin);
  if (!is_pump_on)
  {
    is_pump_on = 1;
    pump_on_seconds = 0;
  }
}

static void turn_pump_off(void)
{
  palClearPad(PORT, pin);
  is_pump_on = 0;
  pump_on_seconds = 0;
}

void pump_receiver_init(void)
{
  pin = config_get_uint(CONFIG_PUMP_RECEIVER_PIN);
  target_node = config_get_uint(CONFIG_PUMP_RECEIVER_TARGET_NODE_ID);
  /* Pins numbered 1..16 */
  if(!pin)
  {
    pin = 16;
    return;
  }
  if(!target_node)
  {
    target_node = 0xFF;
    return;
  }
  pin--;

  palClearPad(PORT, pin);
  palSetPadMode(PORT, pin, PAL_MODE_OUTPUT_PUSHPULL);

  time_since_last_data_seconds = 99;
  pump_on_seconds = 0;
  time_since_last_check_seconds = CHECK_PERIOD_SECONDS + 1;
  target_has_error = 0;
  target_is_full = 1;
  is_pump_on = 0;
}

void pump_receiver_tick(void)
{
  // Turn-off conditions
  if (target_is_full || target_has_error || !pump_receiver_is_up_to_date() || (pump_on_seconds > MAX_PUMP_ON_SECONDS))
  {
    turn_pump_off();
  }
  else if (time_since_last_check_seconds > CHECK_PERIOD_SECONDS)
  {
    time_since_last_check_seconds = 0;

    // hour in UTC
    uint8_t hour = time_hour;

    if (hour >= 5 && hour < 21 && !target_is_full)
    {
      turn_pump_on();
    }
  }

  if (is_pump_on)
  {
    pump_on_seconds++;
  }

  time_since_last_data_seconds++;
  time_since_last_check_seconds++;
}

void on_conduction_sensor_data(CanardRxTransfer* transfer)
{
  if(transfer->payload_len < 1) {
    /* invalid payload */
    return;
  }
  if (transfer->source_node_id != target_node)
  {
    return;
  }

  uint8_t first_byte = transfer->payload_head[0];
  target_has_error = first_byte & TRANSFER_ERROR_MASK;
  target_is_full = first_byte & TRANSFER_TOP_CONDUCTION_SENSOR_MASK;
  time_since_last_data_seconds = 0;
}

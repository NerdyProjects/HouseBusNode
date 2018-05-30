/*
 * pump_receiver.c
 */

#include "node.h"
#include "config.h"
#include "time_data.h"
#include "conduction_sensor.h"

#define STATE_WAIT 0
#define STATE_PUMP 1
#define STATE_REFILL_ONLY 2
#define STATE_REFILL_PUMP 3

#define PUMP_PORT GPIOB
#define MAX_PUMP_ON_SECONDS 60

#define REFILL_PORT GPIOB
#define REFILL_PAD 0

static uint8_t pin;
static uint8_t target_node;
static volatile bool target_has_error = 1;
static volatile bool target_is_full = 1;
static volatile bool target_is_empty = 0;
static volatile bool is_pump_on = 0;
static volatile bool is_refill_on = 0;

static volatile systime_t pump_on_time;
static volatile systime_t refill_on_time;
static volatile systime_t target_data_update_time;

static int state = STATE_WAIT;

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
  return target_data_update_time + TIME_S2I(3) > chVTGetSystemTime();
}

static void turn_pump_on(void)
{
  palSetPad(PUMP_PORT, pin);
  if (!is_pump_on)
  {
    is_pump_on = 1;
    pump_on_time = chVTGetSystemTime();
  }
}

static void turn_pump_off(void)
{
  palClearPad(PUMP_PORT, pin);
  is_pump_on = 0;
}

static void water_refill_start(void)
{
  palSetPad(REFILL_PORT, REFILL_PAD);
  if (!is_refill_on)
  {
    is_refill_on = 1;
    refill_on_time = chVTGetSystemTime();
  }
}

static void water_refill_stop(void)
{
  palClearPad(REFILL_PORT, REFILL_PAD);
  is_refill_on = 0;
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

  palClearPad(PUMP_PORT, pin);
  palSetPadMode(PUMP_PORT, pin, PAL_MODE_OUTPUT_PUSHPULL);
}

void pump_receiver_tick(void)
{
  // IIR low pass filter with 1/x o + (x-1)/x n
  #define IIR_LENGTH 16
  static uint8_t filtered_fill_level = 0;
  static bool source_is_empty = 0;

  uint8_t current_fill_level;
  conduction_evaluate(0, &current_fill_level);
  if (!filtered_fill_level) {
    filtered_fill_level = current_fill_level;
  }
  filtered_fill_level = ((IIR_LENGTH-1)*filtered_fill_level + current_fill_level) / IIR_LENGTH;

  // hysteresis to prevent quick pump toggles
  if (source_is_empty && filtered_fill_level > 245) {
    source_is_empty = 0;
  } else if (!source_is_empty && filtered_fill_level < 230) {
    source_is_empty = 1;
  }

  // state transitions
  int next_state = state;

  switch(state) {
    case(STATE_WAIT):
    {
      if (target_is_empty) {
        if (source_is_empty) {
          next_state = STATE_REFILL_ONLY;
        } else {
          next_state = STATE_PUMP;
        }
      }
      break;
    }
    case(STATE_PUMP):
    {
      if (target_is_full || source_is_empty || pump_on_time + TIME_S2I(MAX_PUMP_ON_SECONDS) < chVTGetSystemTime()) {
        next_state = STATE_WAIT;
      }
      break;
    }
    case(STATE_REFILL_ONLY):
    {
      if (!source_is_empty) {
        next_state = STATE_REFILL_PUMP;
      }
      break;
    }
    case(STATE_REFILL_PUMP):
    {
      if (source_is_empty) {
        next_state = STATE_REFILL_ONLY;
      }
      break;
    }
  }

  // overrides
  if (target_is_full || target_has_error || !pump_receiver_is_up_to_date()) {
    next_state = STATE_WAIT;
  }

  state = next_state;


  // set outputs
  switch(state) {
    case(STATE_WAIT):
    {
      turn_pump_off();
      water_refill_stop();
      break;
    }
    case(STATE_PUMP):
    {
      turn_pump_on();
      water_refill_stop();
      break;
    }
    case(STATE_REFILL_ONLY):
    {
      turn_pump_off();
      water_refill_start();
      break;
    }
    case(STATE_REFILL_PUMP):
    {
      turn_pump_on();
      water_refill_start();
      break;
    }
  }

  uint8_t buffer[HOMEAUTOMATION_GREYWATER_PUMP_STATUS_MESSAGE_SIZE];
  static uint8_t transfer_id = 0;
  canardEncodeScalar(buffer, 0, 8, &state);
  canardLockBroadcast(&canard,
          HOMEAUTOMATION_GREYWATER_PUMP_STATUS_DATA_TYPE_SIGNATURE,
          HOMEAUTOMATION_GREYWATER_PUMP_STATUS_DATA_TYPE_ID, &transfer_id,
          CANARD_TRANSFER_PRIORITY_LOW, buffer, HOMEAUTOMATION_GREYWATER_PUMP_STATUS_MESSAGE_SIZE);
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
  canardDecodeScalar(transfer, 0, 1, NULL, &target_has_error);
  canardDecodeScalar(transfer, 7, 1, NULL, &target_is_full);
  bool target_is_not_empty;
  canardDecodeScalar(transfer, 6, 1, NULL, &target_is_not_empty);
  target_is_empty = !target_is_not_empty;
  target_data_update_time = chVTGetSystemTime();
}

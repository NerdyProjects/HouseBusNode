#include "brightness_data.h"

volatile uint8_t brightness;
static homeautomation_BathroomStatus bathroom_status;

void on_bathroom_data(CanardInstance* ins, CanardRxTransfer* transfer)
{
  if(transfer->payload_len < 1) {
    /* invalid payload */
    return;
  }
  if (transfer->source_node_id != 12)
  {
    return;
  }

  const status = homeautomation_BathroomStatus_decode(
    transfer,
    NULL,
    &bathroom_status,
    NULL
  );
  
  if (status < 0)
  {
    return;
  }

  brightness = bathroom_status.brightness;
}

#include "time_data.h"

volatile uint64_t time_usec;
volatile uint8_t time_hour;

void on_time_data(CanardRxTransfer* transfer)
{
  if(transfer->payload_len < 1) {
    /* invalid payload */
    return;
  }
  if (transfer->source_node_id != 100)
  {
    return;
  }

  canardDecodeScalar(transfer, 0, 56, 0, &time_usec);

  time_hour = (time_usec / (1000ULL*1000ULL*60ULL*60ULL)) % 24ULL;
}

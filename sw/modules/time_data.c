#include "time_data.h"

volatile uint64_t time_usec;
volatile bool time_daylight;
volatile uint8_t time_hour;
volatile uint8_t time_minute;

void on_time_data(CanardInstance* ins, CanardRxTransfer* transfer)
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
  canardDecodeScalar(transfer, 56, 1, 0, &time_daylight);

  time_minute = (time_usec / (1000ULL*1000ULL*60ULL)) % 60ULL;;
  time_hour = (time_usec / (1000ULL*1000ULL*60ULL*60ULL)) % 24ULL;
}

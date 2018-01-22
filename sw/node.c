/*
 * node.c
 *
 *  Created on: 13.01.2018
 *      Author: matthias
 */

#include <hal.h>
#include <ch.h>
#include <canard.h>
#include <drivers/stm32/canard_stm32.h>
#include "uavcan.h"
#include "util.h"

CanardInstance canard;                       ///< The library instance
static uint8_t canard_memory_pool[1024]; ///< Arena for memory allocation, used by the library

/*
 * Node status variables
 */
static uint8_t node_health = UAVCAN_NODE_HEALTH_OK;
static uint8_t node_mode = UAVCAN_NODE_MODE_INITIALIZATION;

static void makeNodeStatusMessage(
    uint8_t buffer[UAVCAN_NODE_STATUS_MESSAGE_SIZE])
{
  memset(buffer, 0, UAVCAN_NODE_STATUS_MESSAGE_SIZE);
  int uptime_sec = ST2S(chVTGetSystemTime());

  /*
   * Here we're using the helper for demonstrational purposes; in this simple case it could be preferred to
   * encode the values manually.
   */
  canardEncodeScalar(buffer, 0, 32, &uptime_sec);
  canardEncodeScalar(buffer, 32, 2, &node_health);
  canardEncodeScalar(buffer, 34, 3, &node_mode);
}

static void readUniqueID(uint8_t* out_uid)
{
  memcpy(out_uid, (void *) UID_BASE, UNIQUE_ID_LENGTH_BYTES);
}

/**
 * This callback is invoked by the library when a new message or request or response is received.
 */
static void onTransferReceived(CanardInstance* ins, CanardRxTransfer* transfer)
{
  if ((transfer->transfer_type == CanardTransferTypeRequest)
      && (transfer->data_type_id == UAVCAN_GET_NODE_INFO_DATA_TYPE_ID))
  {
    uint8_t buffer[UAVCAN_GET_NODE_INFO_RESPONSE_MAX_SIZE];
    memset(buffer, 0, UAVCAN_GET_NODE_INFO_RESPONSE_MAX_SIZE);

    // NodeStatus
    makeNodeStatusMessage(buffer);

    // SoftwareVersion
    buffer[7] = APP_VERSION_MAJOR;
    buffer[8] = APP_VERSION_MINOR;
    buffer[9] = 1;                    // Optional field flags, VCS commit is set
    uint32_t u32 = VCS_COMMIT;
    canardEncodeScalar(buffer, 80, 32, &u32);
    // Image CRC skipped

    // HardwareVersion
    // Major skipped
    // Minor skipped
    readUniqueID(&buffer[24]);
    // Certificate of authenticity skipped

    // Name
    const size_t name_len = strlen(APP_NODE_NAME);
    memcpy(&buffer[41], APP_NODE_NAME, name_len);

    const size_t total_size = 41 + name_len;

    /*
     * Transmitting; in this case we don't have to release the payload because it's empty anyway.
     */
    const int resp_res = canardRequestOrRespond(ins, transfer->source_node_id,
    UAVCAN_GET_NODE_INFO_DATA_TYPE_SIGNATURE,
    UAVCAN_GET_NODE_INFO_DATA_TYPE_ID, &transfer->transfer_id,
        transfer->priority, CanardResponse, &buffer[0], (uint16_t) total_size);
    if (resp_res <= 0)
    {
      ERROR("Could not respond to GetNodeInfo; error %d\n", resp_res);
    }
  }
}

/**
 * This callback is invoked by the library when it detects beginning of a new transfer on the bus that can be received
 * by the local node.
 * If the callback returns true, the library will receive the transfer.
 * If the callback returns false, the library will ignore the transfer.
 * All transfers that are addressed to other nodes are always ignored.
 */
static bool shouldAcceptTransfer(const CanardInstance* ins,
    uint64_t* out_data_type_signature, uint16_t data_type_id,
    CanardTransferType transfer_type, uint8_t source_node_id)
{
  (void) source_node_id;

  if (canardGetLocalNodeID(ins) == CANARD_BROADCAST_NODE_ID)
  {
  }
  else
  {
    if ((transfer_type == CanardTransferTypeRequest)
        && (data_type_id == UAVCAN_GET_NODE_INFO_DATA_TYPE_ID))
    {
      *out_data_type_signature = UAVCAN_GET_NODE_INFO_DATA_TYPE_SIGNATURE;
      return true;
    }
  }

  return false;
}

static void broadcast_node_status(void) {
  uint8_t buffer[UAVCAN_NODE_STATUS_MESSAGE_SIZE];
  makeNodeStatusMessage(buffer);

  static uint8_t transfer_id;

  const int bc_res = canardBroadcast(&canard,
      UAVCAN_NODE_STATUS_DATA_TYPE_SIGNATURE,
      UAVCAN_NODE_STATUS_DATA_TYPE_ID, &transfer_id,
      CANARD_TRANSFER_PRIORITY_LOW, buffer, UAVCAN_NODE_STATUS_MESSAGE_SIZE);
  if (bc_res <= 0)
  {
    ERROR("Could not broadcast node status; error %d\n", bc_res);
  }
}

/**
 * This function is called at 1 Hz rate from the main loop.
 */
static void process1HzTasks(uint64_t timestamp_usec)
{
  /*
   * Purging transfers that are no longer transmitted. This will occasionally free up some memory.
   */
  canardCleanupStaleTransfers(&canard, timestamp_usec);

  /*
   * Printing the memory usage statistics.
   */
  {
    const CanardPoolAllocatorStatistics stats =
        canardGetPoolAllocatorStatistics(&canard);
    const unsigned peak_percent = 100U * stats.peak_usage_blocks
        / stats.capacity_blocks;

    DEBUG(
        "Memory pool stats: capacity %u blocks, usage %u blocks, peak usage %u blocks (%u%%)\n",
        stats.capacity_blocks, stats.current_usage_blocks,
        stats.peak_usage_blocks, peak_percent);

    /*
     * The recommended way to establish the minimal size of the memory pool is to stress-test the application and
     * record the worst case memory usage.
     */
    if (peak_percent > 70)
    {
      DEBUG("WARNING: ENLARGE MEMORY POOL");
    }
  }
  /* Printing can error statistics */
  {
    CanardSTM32Stats stats = canardSTM32GetStats();
    if(stats.error_count > 0 || stats.rx_overflow_count > 0) {
      ERROR("ERR: %ld OVF: %ld\n", stats.error_count, stats.rx_overflow_count);
    }
  }

  /*
   * Transmitting the node status message periodically.
   */
  broadcast_node_status();
  node_mode = UAVCAN_NODE_MODE_OPERATIONAL;


}

static void canDriverEnable(uint8_t enable)
{
  if(enable)
    palClearPad(GPIOC, GPIOC_CAN_SUSPEND);
  else
    palSetPad(GPIOC, GPIOC_CAN_SUSPEND);
}

/**
 * Transmits all frames from the TX queue, receives up to one frame.
 * Returns the number of frames received.
 */
int processTxRxOnce(void)
{
  // Transmitting
  for (const CanardCANFrame* txf = NULL;
      (txf = canardPeekTxQueue(&canard)) != NULL;)
  {
    const int tx_res = canardSTM32Transmit(txf);
    if (tx_res < 0)         // Failure - drop the frame and report
    {
      canardPopTxQueue(&canard);
      DEBUG("Transmit error %d, frame dropped\n", tx_res);
    }
    else if (tx_res > 0)    // Success - just drop the frame
    {
      canardPopTxQueue(&canard);
    }
    else                    // Timeout - just exit and try again later
    {
      break;
    }
  }

  // Receiving
  CanardCANFrame rx_frame;
  const uint64_t timestamp = getMonotonicTimestampUSec();
  const int rx_res = canardSTM32Receive(&rx_frame);
  if (rx_res < 0)             // Failure - report
  {
    DEBUG("Receive error %d\n", rx_res);
  }
  else if (rx_res > 0)        // Success - process the frame
  {
    canardHandleRxFrame(&canard, &rx_frame, timestamp);
    return 1;
  }
  return 0;

}

static void can_enable(void)
{
  rccEnableCAN1(FALSE);
  RCC->APB1RSTR |= (RCC_APB1RSTR_CANRST);
  RCC->APB1RSTR &= ~(RCC_APB1RSTR_CANRST);
}

static THD_WORKING_AREA(waCanThread, 1024);
static THD_FUNCTION(CanThread, arg)
{
  (void) arg;
  chRegSetThreadName("Can");
  systime_t lastInvocation = 0;
  while (true)
  {
    if (chVTTimeElapsedSinceX(lastInvocation) > S2ST(1))
    {
      systime_t currentTime = chVTGetSystemTime();
      process1HzTasks(ST2US_64(currentTime)); /* Copy of LL_ST2US function to preserve 64 bit width */
      lastInvocation = currentTime; /* Intentionally exclude processing time */
    }
    while(processTxRxOnce() == 1) {
      /* send/receive in one block as long as there are packets coming in. */
    }
    /* We are forced to read packets at least once per millisecond (at 250kbps), otherwise we might drop a packet due to RX
     * buffer overflow (hardware fifo with 3 packets)
     */
    chThdSleep(MS2ST(1));

  }
}

int node_init(void)
{
  int res;
  CanardSTM32CANTimings timings;
  canDriverEnable(0);
  can_enable();
  canDriverEnable(1);

  res = canardSTM32ComputeCANTimings(STM32_PCLK, 250000UL, &timings);
  if (res != 0)
  {
    return res;
  }
  res = canardSTM32Init(&timings, CanardSTM32IfaceModeNormal);
  if (res != 0)
  {
    return res;
  }

  /*
   * Initializing the Libcanard instance.
   */
  canardInit(&canard, canard_memory_pool, sizeof(canard_memory_pool),
      onTransferReceived, shouldAcceptTransfer, NULL);
  canardSetLocalNodeID(&canard, 2);
  chThdCreateStatic(waCanThread, sizeof(waCanThread), NORMALPRIO, CanThread,
      NULL);
  return 0;
}

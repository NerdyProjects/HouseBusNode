/*
 * node.c
 *
 *  Created on: 13.01.2018
 *      Author: matthias
 */

#include <hal.h>
#include <ch.h>
#include <canard.h>
#include <string.h>
#include "uavcan.h"
#include "util.h"
#include "node.h"
#include "bme280_node.h"
#include "config.h"
#include "bootloader_interface.h"
#include "firmware_update.h"

CanardInstance canard;                       ///< The library instance
static uint8_t canard_memory_pool[1024]; ///< Arena for memory allocation, used by the library

/* signalled when other threads schedule a TX request */
event_source_t txrequest_event;

uint8_t NodeRestartRequest = 0;

#ifdef BOOTLOADER
uint8_t FirmwareUpdate = 0;
#endif

/*
 * Node status variables
 */

static void makeNodeStatusMessage(
    uint8_t buffer[UAVCAN_NODE_STATUS_MESSAGE_SIZE])
{
  memset(buffer, 0, UAVCAN_NODE_STATUS_MESSAGE_SIZE);
  int uptime_sec = ST2S(chVTGetSystemTime());
  uint8_t node_health = UAVCAN_NODE_HEALTH_OK;
  uint8_t node_mode = UAVCAN_NODE_MODE_OPERATIONAL;
#ifdef BOOTLOADER
  node_mode = UAVCAN_NODE_MODE_MAINTENANCE;
  if(FirmwareUpdate)
    {
      node_mode = UAVCAN_NODE_MODE_SOFTWAREUPDATE;
    }
#endif
  if(NodeRestartRequest)
  {
    node_mode = UAVCAN_NODE_MODE_INITIALIZATION;
  }

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

static void onGetNodeInfo(CanardInstance* ins, CanardRxTransfer* transfer)
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
  const size_t name_len = strlen(nodeconfig.node_name);
  memcpy(&buffer[41], nodeconfig.node_name, name_len);

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
  node_tx_request();
}

static const uint8_t restartNodeMagicNumber[UAVCAN_RESTART_NODE_REQUEST_MAX_SIZE] = {0xAC, 0xCE, 0x55, 0x1B, 0x1E};

static void onRestartNode(CanardInstance* ins, CanardRxTransfer* transfer)
{
  uint8_t response = 0;
  if(memcmp(transfer->payload_head, restartNodeMagicNumber, UAVCAN_RESTART_NODE_REQUEST_MAX_SIZE) == 0)
  {
    response = 1;
    /* Asynchronous restart so we can send response package first.
     * Uses watchdog timeout.
     */
    NodeRestartRequest = 1;
  }
  const int resp_res = canardRequestOrRespond(ins, transfer->source_node_id,
      UAVCAN_RESTART_NODE_DATA_TYPE_SIGNATURE,
      UAVCAN_RESTART_NODE_DATA_TYPE_ID, &transfer->transfer_id,
      transfer->priority, CanardResponse, &response, 1);
}

static void onBeginFirmwareUpdate(CanardInstance* ins, CanardRxTransfer* transfer)
{
  uint8_t response = 0;
  uint8_t filename_length = transfer->payload_len - 1;  /* Tail array optimisation for this data type */
  uint8_t source_id = transfer->payload_head[0];

  if(NodeRestartRequest)
  {
    response = 1;
  }
#ifdef BOOTLOADER
  if(FirmwareUpdate)
  {
    response = 2;
  }
#endif
  if(response == 0)
  {
    bootloader_interface.request_from_node_id = source_id;
    bootloader_interface.request_file_name_length = filename_length;
    for(uint8_t i = 0; i < filename_length; ++i)
    {
      canardDecodeScalar(transfer, 8+8*i, 8, 0, &bootloader_interface.request_file_name[i]);
    }
#ifdef BOOTLOADER
    FirmwareUpdate = 1;
#else
    NodeRestartRequest = 1;
#endif
  }
  canardReleaseRxTransferPayload(ins, transfer);
  const int resp_res = canardRequestOrRespond(ins, transfer->source_node_id,
            UAVCAN_BEGIN_FIRMWARE_UPDATE_DATA_TYPE_SIGNATURE,
            UAVCAN_BEGIN_FIRMWARE_UPDATE_DATA_TYPE_ID, &transfer->transfer_id,
            transfer->priority, CanardResponse, &response, 1);
}

/**
 * This callback is invoked by the library when a new message or request or response is received.
 */
static void onTransferReceived(CanardInstance* ins, CanardRxTransfer* transfer)
{
  if ((transfer->transfer_type == CanardTransferTypeRequest)
      && (transfer->data_type_id == UAVCAN_GET_NODE_INFO_DATA_TYPE_ID))
  {
    onGetNodeInfo(ins, transfer);
  }
  if ((transfer->transfer_type == CanardTransferTypeRequest)
      && (transfer->data_type_id == UAVCAN_RESTART_NODE_DATA_TYPE_ID))
  {
    onRestartNode(ins, transfer);
  }
  if ((transfer->transfer_type == CanardTransferTypeRequest)
      && (transfer->data_type_id == UAVCAN_BEGIN_FIRMWARE_UPDATE_DATA_TYPE_ID))
  {
    onBeginFirmwareUpdate(ins, transfer);
  }
#ifdef BOOTLOADER
  if ((transfer->transfer_type == CanardTransferTypeRequest)
      && (transfer->data_type_id == UAVCAN_FILE_READ_DATA_TYPE_ID))
  {
    onFileRead(ins, transfer);
  }
#endif
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
    if ((transfer_type == CanardTransferTypeRequest)
            && (data_type_id == UAVCAN_BEGIN_FIRMWARE_UPDATE_DATA_TYPE_ID))
    {
      *out_data_type_signature = UAVCAN_BEGIN_FIRMWARE_UPDATE_DATA_TYPE_SIGNATURE;
      return true;
    }
    if ((transfer_type == CanardTransferTypeRequest)
                && (data_type_id == UAVCAN_RESTART_NODE_DATA_TYPE_ID))
    {
      *out_data_type_signature = UAVCAN_RESTART_NODE_DATA_TYPE_SIGNATURE;
      return true;
    }
#ifdef BOOTLOADER
    if ((transfer_type == CanardTransferTypeRequest)
                    && (data_type_id == UAVCAN_FILE_READ_DATA_TYPE_ID))
    {
      *out_data_type_signature = UAVCAN_FILE_READ_DATA_TYPE_SIGNATURE;
      return true;
    }
#endif
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
  node_tx_request();
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

  /*
   * Transmitting the node status message periodically.
   */
  broadcast_node_status();
#ifndef BOOTLOADER
  bme280_node_broadcast_data();
#endif
}

static void canDriverEnable(uint8_t enable)
{
  if(enable)
    palClearPad(GPIOC, GPIOC_CAN_SUSPEND);
  else
    palSetPad(GPIOC, GPIOC_CAN_SUSPEND);
}

/**
 * Transmits all frames from the TX queue.
 * Returns 1 if there is more pending TX to be done, 0 otherwise.
 */
static int processTx(void)
{
  for (const CanardCANFrame* txf = NULL;
      (txf = canardPeekTxQueue(&canard)) != NULL;)
  {
    CANTxFrame txmsg;
    canDriverEnable(1);
    if(txf->id == 0x9e017e82) {
     //__asm volatile("BKPT #0\n");
    }
    chThdSleepMicroseconds(20);
    txmsg.DLC = txf->data_len;
    memcpy(txmsg.data8, txf->data, 8);
    txmsg.EID = txf->id & CANARD_CAN_EXT_ID_MASK;
    txmsg.IDE = 1;
    txmsg.RTR = 0;
    if (canTransmit(&CAND1, CAN_ANY_MAILBOX, &txmsg, TIME_IMMEDIATE) == MSG_OK) {
      canardPopTxQueue(&canard);
      if(!NodeRestartRequest)
      {
        wdgReset(&WDGD1);
      }
    }
    else                    // Timeout - just exit and try again later
    {
      return 1;
    }
  }
  return 0;
}

/* receives all frames from the RX queue */
static void processRx(void)
{
  // Receiving
  CanardCANFrame rx_frame;
  CANRxFrame rxmsg;
  int rx_res;
  while((rx_res = canReceiveTimeout(&CAND1, CAN_ANY_MAILBOX, &rxmsg, TIME_IMMEDIATE)) == MSG_OK)
  {
    const uint64_t timestamp = getMonotonicTimestampUSec();
    memcpy(rx_frame.data, rxmsg.data8, 8);
    rx_frame.data_len = rxmsg.DLC;
    if(rxmsg.IDE) {
      rx_frame.id = CANARD_CAN_FRAME_EFF | rxmsg.EID;
    } else {
      rx_frame.id = rxmsg.SID;
    }
    canardHandleRxFrame(&canard, &rx_frame, timestamp);
  }
}

#define CAN_EVT_RXC 0
#define CAN_EVT_TXC 1
#define CAN_EVT_TXR 2
#define CAN_EVT_ERR 3
static THD_WORKING_AREA(waCanThread, 1024);
static THD_FUNCTION(CanThread, arg)
{
  (void) arg;
  chRegSetThreadName("Can");
  systime_t lastInvocation = 0;
  event_listener_t rxc, txc, txr, err;
  chEvtRegister(&CAND1.rxfull_event, &rxc, CAN_EVT_RXC);
  chEvtRegister(&CAND1.txempty_event, &txc, CAN_EVT_TXC);
  chEvtRegister(&CAND1.error_event, &err, CAN_EVT_ERR);
  chEvtRegister(&txrequest_event, &txr, CAN_EVT_TXR);
  while (true)
  {
#ifdef BOOTLOADER
    if(FirmwareUpdate)
    {
      processFirmwareUpdate();
    }
#endif
    if (chVTTimeElapsedSinceX(lastInvocation) > S2ST(1))
    {
      systime_t currentTime = chVTGetSystemTime();
      process1HzTasks(getMonotonicTimestampUSec());
      lastInvocation = currentTime; /* Intentionally exclude processing time */
    }
    eventmask_t evts = chEvtWaitAnyTimeout(ALL_EVENTS, MS2ST(100));
    if(evts & (1 << CAN_EVT_RXC))
    {
      processRx();
    }
    if((evts & (1 << CAN_EVT_TXC)) || (evts & (1 << CAN_EVT_TXR)))
    {
      processTx();
      if((CAND1.can->TSR & CAN_TSR_TME) == CAN_TSR_TME)
      {
        /*
         * Leave transceiver enabled to be able to send ACK packets.
         * The transceiver might be disabled as soon as we have some always-on nodes
         * on the bus.
         * Anyway, this saves approx. 10 mA.
         */
        //canDriverEnable(0);
      }
    }
    if(evts & (1 << CAN_EVT_ERR))
    {
      int errors = chEvtGetAndClearFlags(&err);
      if(errors & CAN_OVERFLOW_ERROR)
      { /* CAN RX overflow */
        ERROR("CAN RX ovf\n");
      } else if(errors & 0xFFFF0000UL)
      { /* CAN HW error, see CAN_ESR description, in upper 16 bits of errors*/
        ERROR("CAN ERR %x\n", errors);
      }
    }
  }
}

/* Signals a TX request for immediate transmission wakeup.
 * This method can optionally be called after scheduling a transmission,
 * if it is not called, the transmissions will only be handled periodically. */
void node_tx_request(void)
{
  chEvtBroadcast(&txrequest_event);
}

void node_init(void)
{
  /* values for 87.5% samplepoint at 48 MHz and 125000bps */
  static const CANConfig cancfg = {
    CAN_MCR_ABOM | CAN_MCR_AWUM | CAN_MCR_TXFP,
    CAN_BTR_SJW(0) | CAN_BTR_TS2(1) |
    CAN_BTR_TS1(12) | CAN_BTR_BRP(23)
  };

  canDriverEnable(1);
  wdgReset(&WDGD1);
  canStart(&CAND1, &cancfg);

  chEvtObjectInit(&txrequest_event);

  /*
   * Initializing the Libcanard instance.
   */
  wdgReset(&WDGD1);
  canardInit(&canard, canard_memory_pool, sizeof(canard_memory_pool),
      onTransferReceived, shouldAcceptTransfer, NULL);
  canardSetLocalNodeID(&canard, 2);
  wdgReset(&WDGD1);
#ifdef BOOTLOADER
  if(bootloader_interface.request_from_node_id && bootloader_interface.request_file_name_length)
  {
    FirmwareUpdate = 1;
  }
#endif
  chThdCreateStatic(waCanThread, sizeof(waCanThread), HIGHPRIO, CanThread,
      NULL);
}

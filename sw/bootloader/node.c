/*
 * node.c
 *
 *  Created on: 13.01.2018
 *      Author: matthias
 */

#include <hal.h>
#include "ch.h"
#include <canard.h>
#include <string.h>
#include "uavcan.h"
#include "node.h"
#include "../bootloader_interface.h"
#include "firmware_update.h"
#include "bootloader.h"

CanardInstance canard;                       ///< The library instance
static uint8_t canard_memory_pool[1024]; ///< Arena for memory allocation, used by the library

/* signalled when other threads schedule a TX request */
event_source_t txrequest_event;

systime_t NodeRestartAt;

static uint8_t node_health = UAVCAN_NODE_HEALTH_OK;

volatile uint8_t FirmwareUpdate = 0;

/* ChibiOs wrappers around canard to be able to have indepentent RX/TX Threads */
MUTEX_DECL(canardMtx);

#define canardLock(X) chMtxLock(&canardMtx); \
  X; \
  chMtxUnlock(&canardMtx)

int canardLockRequestOrRespond(CanardInstance* ins,             ///< Library instance
                           uint8_t destination_node_id,     ///< Node ID of the server/client
                           uint64_t data_type_signature,    ///< See above
                           uint8_t data_type_id,            ///< Refer to the specification
                           uint8_t* inout_transfer_id,      ///< Pointer to a persistent variable with the transfer ID
                           uint8_t priority,                ///< Refer to definitions CANARD_TRANSFER_PRIORITY_*
                           CanardRequestResponse kind,      ///< Refer to CanardRequestResponse
                           const void* payload,             ///< Transfer payload
                           uint16_t payload_len)           ///< Length of the above, in bytes
{
  int res;
  chMtxLock(&canardMtx);
  res = canardRequestOrRespond(ins, destination_node_id, data_type_signature, data_type_id, inout_transfer_id,
      priority, kind, payload, payload_len);
  chMtxUnlock(&canardMtx);
  node_tx_request();
  return res;
}


int canardLockBroadcast(CanardInstance* ins,            ///< Library instance
                    uint64_t data_type_signature,   ///< See above
                    uint16_t data_type_id,          ///< Refer to the specification
                    uint8_t* inout_transfer_id,     ///< Pointer to a persistent variable containing the transfer ID
                    uint8_t priority,               ///< Refer to definitions CANARD_TRANSFER_PRIORITY_*
                    const void* payload,            ///< Transfer payload
                    uint16_t payload_len)          ///< Length of the above, in bytes
{
  int res;
  chMtxLock(&canardMtx);
  res = canardBroadcast(ins, data_type_signature, data_type_id, inout_transfer_id, priority, payload, payload_len);
  chMtxUnlock(&canardMtx);
  node_tx_request();
  return res;
}


void requestNodeRestart(void)
{
  if(NodeRestartAt == 0)
  {
    NodeRestartAt = chVTGetSystemTimeX() + TIME_S2I(1);
  }
}

static uint8_t isNodeRestartRequested(void)
{
  return (NodeRestartAt != 0);
}

static void makeNodeStatusMessage(
    uint8_t buffer[UAVCAN_NODE_STATUS_MESSAGE_SIZE])
{
  memset(buffer, 0, UAVCAN_NODE_STATUS_MESSAGE_SIZE);
  int uptime_sec = TIME_I2S(chVTGetSystemTimeX());
  uint16_t vdda = 0;
  uint8_t node_mode = UAVCAN_NODE_MODE_MAINTENANCE;
  if(FirmwareUpdate)
  {
    node_mode = UAVCAN_NODE_MODE_SOFTWAREUPDATE;
  }
  if(isNodeRestartRequested())
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
  canardEncodeScalar(buffer, 40, 16, &vdda);
}

static void readUniqueID(uint8_t* out_uid)
{
  memcpy(out_uid, (void *) UID_BASE, UNIQUE_ID_LENGTH_BYTES);
}

static void onGetNodeInfo(CanardInstance* ins, CanardRxTransfer* transfer)
{
  uint8_t name[] = "Bootloader Node";
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
  memcpy(&buffer[41], name, sizeof(name));

  const size_t total_size = 41 + sizeof(name);

  /*
   * Transmitting; in this case we don't have to release the payload because it's empty anyway.
   */
  const int resp_res = canardLockRequestOrRespond(ins, transfer->source_node_id,
  UAVCAN_GET_NODE_INFO_DATA_TYPE_SIGNATURE,
  UAVCAN_GET_NODE_INFO_DATA_TYPE_ID, &transfer->transfer_id,
      transfer->priority, CanardResponse, &buffer[0], (uint16_t) total_size);
  if (resp_res <= 0)
  {
    ERROR("GetNodeInfo resp fail; error %d\n", resp_res);
  }
}

static const uint8_t restartNodeMagicNumber[UAVCAN_RESTART_NODE_REQUEST_MAX_SIZE] = {0x1E, 0x1B, 0x55, 0xCE, 0xAC};

static void onRestartNode(CanardInstance* ins, CanardRxTransfer* transfer)
{
  uint8_t response = 0;
  if(memcmp(transfer->payload_head, restartNodeMagicNumber, UAVCAN_RESTART_NODE_REQUEST_MAX_SIZE) == 0)
  {
    response = 1;
    /* Asynchronous restart so we can send response package first.
     */
    requestNodeRestart();
  }
  const int resp_res = canardLockRequestOrRespond(ins, transfer->source_node_id,
      UAVCAN_RESTART_NODE_DATA_TYPE_SIGNATURE,
      UAVCAN_RESTART_NODE_DATA_TYPE_ID, &transfer->transfer_id,
      transfer->priority, CanardResponse, &response, 1);
}

static void onBeginFirmwareUpdate(CanardInstance* ins, CanardRxTransfer* transfer)
{
  uint8_t response = 0;
  uint8_t filename_length = transfer->payload_len - 1;  /* Tail array optimisation for this data type */
  uint8_t source_id = transfer->payload_head[0];

  if(isNodeRestartRequested())
  {
    response = 1;
  }
  if(FirmwareUpdate)
  {
    response = 2;
  }
  if(response == 0)
  {
    bootloader_interface.request_from_node_id = source_id;
    bootloader_interface.request_file_name_length = filename_length;
    for(uint8_t i = 0; i < filename_length; ++i)
    {
      canardDecodeScalar(transfer, 8+8*i, 8, 0, &bootloader_interface.request_file_name[i]);
    }
    FirmwareUpdate = 1;
  }
  canardLock(canardReleaseRxTransferPayload(ins, transfer));
  const int resp_res = canardLockRequestOrRespond(ins, transfer->source_node_id,
            UAVCAN_BEGIN_FIRMWARE_UPDATE_DATA_TYPE_SIGNATURE,
            UAVCAN_BEGIN_FIRMWARE_UPDATE_DATA_TYPE_ID, &transfer->transfer_id,
            transfer->priority, CanardResponse, &response, 1);
}

typedef struct {
  uint8_t transfer_type;
  uint16_t data_type_id;
  uint64_t data_type_signature;
  OnTransferReceivedCB cb;
} ReceiveTransfer;

#undef REGISTER_TRANSFER
#define REGISTER_TRANSFER(type, id, signature, cb) {type, id, signature, cb},
ReceiveTransfer receiveTransfers[] = {
REGISTER_TRANSFER(CanardTransferTypeRequest, UAVCAN_GET_NODE_INFO_DATA_TYPE_ID, UAVCAN_GET_NODE_INFO_DATA_TYPE_SIGNATURE, onGetNodeInfo)
REGISTER_TRANSFER(CanardTransferTypeRequest, UAVCAN_RESTART_NODE_DATA_TYPE_ID, UAVCAN_RESTART_NODE_DATA_TYPE_SIGNATURE, onRestartNode)
{CanardTransferTypeRequest, UAVCAN_BEGIN_FIRMWARE_UPDATE_DATA_TYPE_ID, UAVCAN_BEGIN_FIRMWARE_UPDATE_DATA_TYPE_SIGNATURE, onBeginFirmwareUpdate}
, {CanardTransferTypeResponse, UAVCAN_FILE_READ_DATA_TYPE_ID, UAVCAN_FILE_READ_DATA_TYPE_SIGNATURE, onFileRead}
};

/**
 * This callback is invoked by the library when a new message or request or response is received.
 */
static void onTransferReceived(CanardInstance* ins, CanardRxTransfer* transfer)
{
  uint8_t i;
  for (i = 0; i < sizeof(receiveTransfers) / sizeof(receiveTransfers[0]); ++i)
  {
    if ((transfer->transfer_type == receiveTransfers[i].transfer_type)
      && (transfer->data_type_id == receiveTransfers[i].data_type_id))
    {
      receiveTransfers[i].cb(ins, transfer);
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
    uint8_t i;
    for (i = 0; i < sizeof(receiveTransfers) / sizeof(receiveTransfers[0]); ++i)
    {
      if ((transfer_type == receiveTransfers[i].transfer_type)
        && (data_type_id == receiveTransfers[i].data_type_id))
      {
        *out_data_type_signature = receiveTransfers[i].data_type_signature;
        return true;
      }
    }
  }

  return false;
}

static void broadcast_node_status(void) {
  uint8_t buffer[UAVCAN_NODE_STATUS_MESSAGE_SIZE];
  makeNodeStatusMessage(buffer);

  static uint8_t transfer_id;

  const int bc_res = canardLockBroadcast(&canard,
      UAVCAN_NODE_STATUS_DATA_TYPE_SIGNATURE,
      UAVCAN_NODE_STATUS_DATA_TYPE_ID, &transfer_id,
      CANARD_TRANSFER_PRIORITY_LOW, buffer, UAVCAN_NODE_STATUS_MESSAGE_SIZE);
  if (bc_res <= 0)
  {
    ERROR("node status bc fail; error %d\n", bc_res);
  }
}

/**
 * This function is called at 1 Hz rate from the main loop.
 */
static void process1HzTasks(uint32_t timestamp)
{
  /*
   * Purging transfers that are no longer transmitted. This will occasionally free up some memory.
   */
  canardLock(canardCleanupStaleTransfers(&canard, timestamp));

  /*
   * Transmitting the node status message periodically.
   */
  broadcast_node_status();
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
  chMtxLock(&canardMtx);
  for (const CanardCANFrame* txf = NULL;
      (txf = canardPeekTxQueue(&canard)) != NULL;)
  {
    CANTxFrame txmsg;
    txmsg.DLC = txf->data_len;
    memcpy(txmsg.data8, txf->data, 8);
    txmsg.EID = txf->id & CANARD_CAN_EXT_ID_MASK;
    txmsg.IDE = 1;
    txmsg.RTR = 0;
    if (canTransmit(&CAND1, CAN_ANY_MAILBOX, &txmsg, TIME_IMMEDIATE) == MSG_OK) {
      wdgReset(&WDGD1);
      canardPopTxQueue(&canard);
    }
    else                    // Timeout - just exit and try again later
    {
      chMtxUnlock(&canardMtx);
      return 1;
    }
  }
  chMtxUnlock(&canardMtx);
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
    const uint32_t timestamp = getMonotonicTimestamp();
    memcpy(rx_frame.data, rxmsg.data8, 8);
    rx_frame.data_len = rxmsg.DLC;
    if(rxmsg.IDE) {
      rx_frame.id = CANARD_CAN_FRAME_EFF | rxmsg.EID;
    } else {
      rx_frame.id = rxmsg.SID;
    }
    canardLock(canardHandleRxFrame(&canard, &rx_frame, timestamp));
  }
}

#define CAN_EVT_RXC 0
#define CAN_EVT_TXC 1
#define CAN_EVT_TXR 2
#define CAN_EVT_ERR 3
static THD_WORKING_AREA(waCanNodeThread, 2048);
static THD_FUNCTION(CanNodeThread, arg)
{
  (void) arg;
  chRegSetThreadName("CanNode");
  systime_t lastInvocation = 0;
  event_listener_t txc, txr, err;
  chEvtRegister(&CAND1.txempty_event, &txc, CAN_EVT_TXC);
  chEvtRegister(&CAND1.error_event, &err, CAN_EVT_ERR);
  chEvtRegister(&txrequest_event, &txr, CAN_EVT_TXR);
  while (true)
  {
    if(FirmwareUpdate)
    {
      int res;
      res = processFirmwareUpdate(&canard);
      if(res == FIRMWARE_UPDATE_ERR_FLASH_FAILED)
      {
        /* Firmware update failed - application is likely to be broken now */
        node_health = UAVCAN_NODE_HEALTH_CRITICAL;
        FirmwareUpdate = 0;
        bootloader_interface.request_from_node_id = 0;
      } else if(res == FIRMWARE_UPDATE_DONE_SUCCESS)
      {
        bootloader_interface.request_from_node_id = 0;
        FirmwareUpdate = 0;
      }
    }
    if (chVTTimeElapsedSinceX(lastInvocation) >= TIME_S2I(1))
    {
      systime_t currentTime = chVTGetSystemTimeX();
      process1HzTasks(getMonotonicTimestamp());
      lastInvocation = currentTime; /* Intentionally exclude processing time */
      if(NodeRestartAt && (currentTime > NodeRestartAt))
      {
        NVIC_SystemReset();
      }
    }
    eventmask_t evts = chEvtWaitAnyTimeout(ALL_EVENTS, TIME_MS2I(100));
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

static THD_WORKING_AREA(waCanRxThread, 2048);
static THD_FUNCTION(CanRxThread, arg)
{
  (void) arg;
  chRegSetThreadName("CanRx");
  event_listener_t rxc;
  chEvtRegister(&CAND1.rxfull_event, &rxc, CAN_EVT_RXC);
  while(1)
  {
    eventmask_t evts = chEvtWaitAnyTimeout(ALL_EVENTS, TIME_MS2I(100));
    if(evts & (1 << CAN_EVT_RXC))
    {
      processRx();
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


void node_init(uint8_t node_id)
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
  canardSetLocalNodeID(&canard, node_id);
  wdgReset(&WDGD1);
  chThdCreateStatic(waCanRxThread, sizeof(waCanRxThread), HIGHPRIO, CanRxThread,
        NULL);
  chThdCreateStatic(waCanNodeThread, sizeof(waCanNodeThread), NORMALPRIO, CanNodeThread,
      NULL);
}

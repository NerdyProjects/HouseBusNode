/*
 * node.c
 *
 *  Created on: 13.01.2018
 *      Author: matthias
 */

#include <hal.h>
#include "ch.h"
#include "chprintf.h"
#include <canard.h>
#include <string.h>
#include "uavcan.h"
#include "util.h"
#include "node.h"
#include "config.h"
#include "bootloader_interface.h"
#include "config.h"
#include "drivers/analog.h"
#include "dimmer.h"
#include "nodes/node.h"
#include "modules/modules.h"

CanardInstance canard;                       ///< The library instance
static uint8_t canard_memory_pool[1024]; ///< Arena for memory allocation, used by the library

/* signalled when other threads schedule a TX request */
event_source_t txrequest_event;

systime_t NodeRestartAt;

static uint8_t node_health = UAVCAN_NODE_HEALTH_OK;
static uint8_t node_mode = UAVCAN_NODE_MODE_INITIALIZATION;

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
  bootloader_interface.node_id = config_get_uint(CONFIG_NODE_ID);
  bootloader_interface.magic = BOOTLOADER_INTERFACE_VALID_MAGIC;
  if(NodeRestartAt == 0)
  {
    NodeRestartAt = chVTGetSystemTimeX() + TIME_S2I(1);
  }
}

uint8_t node_getMode(void)
{
  return node_mode;
}

void node_setMode(uint8_t mode)
{
  node_mode = mode;
}

static uint8_t isNodeRestartRequested(void)
{
  return (NodeRestartAt != 0);
}

static void payloadExtractArray(CanardRxTransfer *transfer, uint16_t offset, uint8_t len, uint8_t *dst)
{
  while(len)
  {
    canardDecodeScalar(transfer, offset*8, 8, 0, dst++);
    len--;
    offset++;
  }
}

static void makeNodeStatusMessage(
    uint8_t buffer[UAVCAN_NODE_STATUS_MESSAGE_SIZE])
{
  memset(buffer, 0, UAVCAN_NODE_STATUS_MESSAGE_SIZE);
  int uptime_sec = TIME_I2S(chVTGetSystemTimeX());
  uint16_t vdda = 0;
  vdda= analog_get_vdda();
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
  uint8_t name_len;
  config_get(CONFIG_NODE_NAME, &buffer[41], &name_len);

  const size_t total_size = 41 + name_len;

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

static void onParamGetSet(CanardInstance* ins, CanardRxTransfer* transfer)
{
  uint8_t buffer[UAVCAN_PARAM_GETSET_RESPONSE_MAX_SIZE];
  uint16_t response_size;
  memset(buffer, 0, UAVCAN_PARAM_GETSET_RESPONSE_MAX_SIZE);

  if(transfer->payload_len < 2) {
    /* invalid payload */
    return;
  }

  uint16_t config_id;
  uint8_t unionTag;
  uint8_t value_len;
  int name_len;
  canardDecodeScalar(transfer, 0, 13, 0, &config_id);
  canardDecodeScalar(transfer, 13, 3, 0, &unionTag);
  switch(unionTag)
  {
  case 0:
    /* empty */
    value_len = 0;
    break;
  case 1:
    /* int64 */
    value_len = 8;
    break;
  case 2:
    /* float32 */
    value_len = 4;
    break;
  case 3:
    /* 8 bit boolean */
    value_len = 1;
    break;
  case 4:
    /* 4: string with length prefix */
    value_len = transfer->payload_head[2] + 1;
    break;
  default:
    /* invalid payload */
    return;
  }

  name_len = transfer->payload_len - 2 - value_len;
  if(name_len < 0) {
    /* invalid payload */
    return;
  } else if(name_len > 0)
  {
    uint8_t name[92];
    payloadExtractArray(transfer, 2 + value_len, name_len, name);
    int id = config_get_id_by_name(name, name_len);
    if(id < 0)
    {
      /* merge invalid ID paths using an invalid 16 bit ID */
      config_id = 65535;
    } else
    {
      config_id = (uint16_t)id;
    }
  }

  if(config_get_param_size(config_id) != 0)
  {
    /* Parameter exists - Perform get or set */
    if(unionTag != 0)
    {
      /* Set */
      if(unionTag == 1)
      {
        /* int64*/
        int64_t value;
        canardDecodeScalar(transfer, 16, 64, 1, &value);
        config_set(config_id, &value, 8);
      } else if(unionTag == 4)
      {
        /* string */
        uint8_t str[128];
        payloadExtractArray(transfer, 3, transfer->payload_head[2], str);
        config_set(config_id, str, transfer->payload_head[2]);
      } else if(unionTag == 2)
      {
        float value;
        canardDecodeScalar(transfer, 16, 32, 1, &value);
        config_set(config_id, &value, 4);
      }
      app_config_update();
    }
    /* Always perform get for response: Actual/default/min/max value */
    switch (config_get_param_type(config_id))
    {
    case CONFIG_PARAM_STRING:
      buffer[0] = 4;
      config_get(config_id, &buffer[2], &buffer[1]);
      response_size = 1 + 1 + buffer[1] + 3;
      break;
    case CONFIG_PARAM_INT:
      buffer[0] = 1;
      config_get(config_id, &buffer[1], NULL);
      response_size = 1 + 8 + 3;
      break;
    case CONFIG_PARAM_FLOAT32:
      buffer[0] = 2;
      config_get(config_id, &buffer[1], NULL);
      response_size = 1 + 4 + 3;
      break;
    default:
      util_assert(0);
      break;
    }
    /* read name */
    config_get_name(config_id, &buffer[response_size]);
    response_size += strlen(&buffer[response_size]);
  } else
  {
    /* Parameter does not exist - response with 4 empty values signals that */
    response_size = 4;
  }
  canardLock(canardReleaseRxTransferPayload(ins, transfer));
  const int resp_res = canardLockRequestOrRespond(ins, transfer->source_node_id,
            UAVCAN_PARAM_GETSET_DATA_TYPE_SIGNATURE,
            UAVCAN_PARAM_GETSET_DATA_TYPE_ID, &transfer->transfer_id,
            transfer->priority, CanardResponse, buffer, response_size);
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
  if(response == 0)
  {
    bootloader_interface.request_from_node_id = source_id;
    bootloader_interface.request_file_name_length = filename_length;
    for(uint8_t i = 0; i < filename_length; ++i)
    {
      canardDecodeScalar(transfer, 8+8*i, 8, 0, &bootloader_interface.request_file_name[i]);
    }
    requestNodeRestart();
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


#define REGISTER_TRANSFER(type, id, signature, cb) {type, id, signature, cb},
ReceiveTransfer receiveTransfers[] = {
#include "transfer_registrations.h"
REGISTER_TRANSFER(CanardTransferTypeRequest, UAVCAN_GET_NODE_INFO_DATA_TYPE_ID, UAVCAN_GET_NODE_INFO_DATA_TYPE_SIGNATURE, onGetNodeInfo)
REGISTER_TRANSFER(CanardTransferTypeRequest, UAVCAN_RESTART_NODE_DATA_TYPE_ID, UAVCAN_RESTART_NODE_DATA_TYPE_SIGNATURE, onRestartNode)
REGISTER_TRANSFER(CanardTransferTypeRequest, UAVCAN_PARAM_GETSET_DATA_TYPE_ID, UAVCAN_PARAM_GETSET_DATA_TYPE_SIGNATURE, onParamGetSet)
{CanardTransferTypeRequest, UAVCAN_BEGIN_FIRMWARE_UPDATE_DATA_TYPE_ID, UAVCAN_BEGIN_FIRMWARE_UPDATE_DATA_TYPE_SIGNATURE, onBeginFirmwareUpdate}
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
   * Printing the memory usage statistics.
   */
  {
    canardLock(
    const CanardPoolAllocatorStatistics stats =
        canardGetPoolAllocatorStatistics(&canard));
    const unsigned peak_percent = 100U * stats.peak_usage_blocks
        / stats.capacity_blocks;

    DEBUG(
        "Canard mem: cap %u blks, using %u, pk %u (%u%%)\n",
        stats.capacity_blocks, stats.current_usage_blocks,
        stats.peak_usage_blocks, peak_percent);

    /*
     * The recommended way to establish the minimal size of the memory pool is to stress-test the application and
     * record the worst case memory usage.
     */
    if (peak_percent > 70)
    {
      DEBUG("WARN: ENLARGE Canard MEM");
    }
  }

  /*
   * Transmitting the node status message periodically.
   */
  broadcast_node_status();
  if(node_mode == UAVCAN_NODE_MODE_OPERATIONAL)
  {
    app_tick();
  }
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
    if (chVTTimeElapsedSinceX(lastInvocation) > TIME_S2I(1))
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

static THD_WORKING_AREA(waFastTasksThread, 1024);
static THD_FUNCTION(FastTasksThread, arg)
{
  (void) arg;
  chRegSetThreadName("FastTasks");
  while(node_getMode() != UAVCAN_NODE_MODE_OPERATIONAL)
  {
    chThdSleepS(5);
  }
  systime_t nextInvocation = chVTGetSystemTimeX();
  while(node_getMode() == UAVCAN_NODE_MODE_OPERATIONAL)
  {
    /* Executed every ~5ms. Can be used for key debouncing etc. */
    if(dimmer_is_present())
    {
      dimmer_tick();
    }
    app_fast_tick();
    nextInvocation = chThdSleepUntilWindowed(nextInvocation, nextInvocation + TIME_MS2I(5));
  }
}

/* Signals a TX request for immediate transmission wakeup.
 * This method can optionally be called after scheduling a transmission,
 * if it is not called, the transmissions will only be handled periodically. */
void node_tx_request(void)
{
  chEvtBroadcast(&txrequest_event);
}


/* This message unfortunately needs ~150 bytes of stack. Please be careful when using!
 *
 */
void node_debug(uint8_t loglevel, const char *source, const char *msg)
{
  uint8_t buffer[UAVCAN_DEBUG_LOG_MESSAGE_MESSAGE_SIZE];
  uint8_t source_len = strlen(source) & 31;
  uint8_t msg_len = strlen(msg);
  static uint8_t transfer_id;
  if(msg_len > 90)
  {
    msg_len = 90;
  }
  /* Use manual mutex locking here to lock the global buffer as well */
  chMtxLock(&canardMtx);
  buffer[0] = (loglevel << 5) | source_len;
  memcpy(&buffer[1], source, source_len);
  memcpy(&buffer[1+source_len], msg, msg_len);
  canardBroadcast(
      &canard,
      UAVCAN_DEBUG_LOG_MESSAGE_DATA_TYPE_SIGNATURE,
      UAVCAN_DEBUG_LOG_MESSAGE_DATA_TYPE_ID,
      &transfer_id,
      CANARD_TRANSFER_PRIORITY_LOWEST,
      buffer, 1 + source_len + msg_len
      );
  chMtxUnlock(&canardMtx);
}

void node_debug_int(uint8_t loglevel, const char *source, int32_t number)
{
  char dbgbuf[12];
  chsnprintf(dbgbuf, 12, "%d", number);
  node_debug(loglevel, source, dbgbuf);
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
  canardSetLocalNodeID(&canard, config_get_uint(CONFIG_NODE_ID));
  wdgReset(&WDGD1);
  chThdCreateStatic(waCanRxThread, sizeof(waCanRxThread), HIGHPRIO, CanRxThread,
        NULL);
  chThdCreateStatic(waCanNodeThread, sizeof(waCanNodeThread), NORMALPRIO, CanNodeThread,
      NULL);
  chThdCreateStatic(waFastTasksThread, sizeof(waFastTasksThread), HIGHPRIO, FastTasksThread,
        NULL);
}

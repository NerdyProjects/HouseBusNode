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
#include "config.h"
#include "drivers/analog.h"
#ifdef BOOTLOADER
#include "bootloader.h"
#endif

CanardInstance canard;                       ///< The library instance
static uint8_t canard_memory_pool[1024]; ///< Arena for memory allocation, used by the library

/* signalled when other threads schedule a TX request */
event_source_t txrequest_event;

systime_t NodeRestartAt;


static uint8_t node_health = UAVCAN_NODE_HEALTH_OK;

#ifdef BOOTLOADER
volatile uint8_t FirmwareUpdate = 0;
#endif

void requestNodeRestart(void)
{
  if(NodeRestartAt == 0)
  {
    NodeRestartAt = chVTGetSystemTime() + S2ST(1);
  }
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
  int uptime_sec = ST2S(chVTGetSystemTime());
  uint8_t node_mode = UAVCAN_NODE_MODE_OPERATIONAL;
#ifdef BOOTLOADER
  node_mode = UAVCAN_NODE_MODE_MAINTENANCE;
  if(FirmwareUpdate)
    {
      node_mode = UAVCAN_NODE_MODE_SOFTWAREUPDATE;
    }
#endif
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
  const int resp_res = canardRequestOrRespond(ins, transfer->source_node_id,
  UAVCAN_GET_NODE_INFO_DATA_TYPE_SIGNATURE,
  UAVCAN_GET_NODE_INFO_DATA_TYPE_ID, &transfer->transfer_id,
      transfer->priority, CanardResponse, &buffer[0], (uint16_t) total_size);
  if (resp_res <= 0)
  {
    ERROR("GetNodeInfo resp fail; error %d\n", resp_res);
  }
  node_tx_request();
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
  const int resp_res = canardRequestOrRespond(ins, transfer->source_node_id,
      UAVCAN_RESTART_NODE_DATA_TYPE_SIGNATURE,
      UAVCAN_RESTART_NODE_DATA_TYPE_ID, &transfer->transfer_id,
      transfer->priority, CanardResponse, &response, 1);
}

static void onParamGetSet(CanardInstance* ins, CanardRxTransfer* transfer)
{
  uint8_t buffer[UAVCAN_PARAM_GETSET_RESPONSE_MAX_SIZE];
  uint16_t response_size;
  memset(buffer, 0, UAVCAN_PARAM_GETSET_RESPONSE_MAX_SIZE);
  #ifdef BOOTLOADER
  bootloader_command_executed();
  #endif


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
      }
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
  canardReleaseRxTransferPayload(ins, transfer);
  const int resp_res = canardRequestOrRespond(ins, transfer->source_node_id,
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
#ifdef BOOTLOADER
  bootloader_command_executed();
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
    requestNodeRestart();
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
  if ((transfer->transfer_type == CanardTransferTypeRequest)
        && (transfer->data_type_id == UAVCAN_PARAM_GETSET_DATA_TYPE_ID))
  {
    onParamGetSet(ins, transfer);
  }
#ifdef BOOTLOADER
  if ((transfer->transfer_type == CanardTransferTypeResponse)
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
    if ((transfer_type == CanardTransferTypeRequest)
                && (data_type_id == UAVCAN_PARAM_GETSET_DATA_TYPE_ID))
    {
      *out_data_type_signature = UAVCAN_PARAM_GETSET_DATA_TYPE_SIGNATURE;
      return true;
    }
#ifdef BOOTLOADER
    if ((transfer_type == CanardTransferTypeResponse)
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
    ERROR("node status bc fail; error %d\n", bc_res);
  }
  node_tx_request();
}


/* Valid flags: 1 - humidity valid, 2 - pressure valid */
static void broadcast_environment_data(int32_t centiCelsiusTemperature, uint32_t milliRelativeHumidity, uint32_t centiBarPressure, uint8_t validFlags)
{
  uint8_t buffer[HOMEAUTOMATION_ENVIRONMENT_MESSAGE_SIZE];
  uint8_t transfer_id = 0;

  /* data contains:
   *  humidity in millipercent (percent = hum / 1000) [0..100000] -> 17 bit
   *  pressure in 10^-2 mbar (mbar = pres / 100) [30000-110000] -> 18 bit
   *  temperature in centidegrees (degree = temp / 100) [-4000..85000] -> 19 bit
   */
  memset(buffer, 0, HOMEAUTOMATION_ENVIRONMENT_MESSAGE_SIZE);
  canardEncodeScalar(buffer, 0, 2, &validFlags);
  canardEncodeScalar(buffer, 2, 19, &centiCelsiusTemperature);
  canardEncodeScalar(buffer, 21, 17, &milliRelativeHumidity);
  canardEncodeScalar(buffer, 38, 18, &centiBarPressure);
  const int bc_res = canardBroadcast(&canard,
        HOMEAUTOMATION_ENVIRONMENT_DATA_TYPE_SIGNATURE,
        HOMEAUTOMATION_ENVIRONMENT_DATA_TYPE_ID, &transfer_id,
        CANARD_TRANSFER_PRIORITY_LOW, buffer, HOMEAUTOMATION_ENVIRONMENT_MESSAGE_SIZE);
  if (bc_res <= 0)
  {
    ERROR("failed environment bc; error %d\n", bc_res);
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
#ifndef BOOTLOADER
  if(bme280_is_present())
  {
    struct bme280_data data;
    if(bme280_node_read(&data) == 0)
    {
      broadcast_environment_data(data.temperature, data.humidity, data.pressure, 3);
    }
  } else
  {
    broadcast_environment_data(analog_get_internal_ts(), 0, 0, 0);
  }
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
      wdgReset(&WDGD1);
      canardPopTxQueue(&canard);
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
static THD_WORKING_AREA(waCanThread, 2048);
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
      int res;
      res = processFirmwareUpdate(&canard);
      if(res == FIRMWARE_UPDATE_ERR_FLASH_FAILED)
      {
        /* Firmware update failed - application is likely to be broken now */
        node_health = UAVCAN_NODE_HEALTH_CRITICAL;
        FirmwareUpdate = 0;
      } else if(res == FIRMWARE_UPDATE_DONE_SUCCESS)
      {
        FirmwareUpdate = 0;
      }
    }
#endif
    if (chVTTimeElapsedSinceX(lastInvocation) > S2ST(1))
    {
      systime_t currentTime = chVTGetSystemTime();
      process1HzTasks(getMonotonicTimestampUSec());
      lastInvocation = currentTime; /* Intentionally exclude processing time */
      if(NodeRestartAt && (currentTime > NodeRestartAt))
      {
        NVIC_SystemReset();
      }
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
  canardSetLocalNodeID(&canard, config_get_uint(CONFIG_NODE_ID));
  wdgReset(&WDGD1);
  chThdCreateStatic(waCanThread, sizeof(waCanThread), HIGHPRIO, CanThread,
      NULL);
}

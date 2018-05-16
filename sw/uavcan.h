/*
 * uavcan.h
 *
 *  Created on: 13.01.2018
 *      Author: matthias
 */

#ifndef UAVCAN_H_
#define UAVCAN_H_

 /* Some useful constants defined by the UAVCAN specification.
 * Data type signature values can be easily obtained with the script show_data_type_info.py
 */
#define UAVCAN_NODE_ID_ALLOCATION_DATA_TYPE_ID                      1
#define UAVCAN_NODE_ID_ALLOCATION_DATA_TYPE_SIGNATURE               0x0b2a812620a11d40
#define UAVCAN_NODE_ID_ALLOCATION_RANDOM_TIMEOUT_RANGE_USEC         400000UL
#define UAVCAN_NODE_ID_ALLOCATION_REQUEST_DELAY_OFFSET_USEC         600000UL

#define UAVCAN_NODE_STATUS_MESSAGE_SIZE                             7
#define UAVCAN_NODE_STATUS_DATA_TYPE_ID                             341
#define UAVCAN_NODE_STATUS_DATA_TYPE_SIGNATURE                      0x0f0868d0c1a7c6f1

enum uavcan_node_health {UAVCAN_NODE_HEALTH_OK,
  UAVCAN_NODE_HEALTH_WARNING,
  UAVCAN_NODE_HEALTH_ERROR,
  UAVCAN_NODE_HEALTH_CRITICAL
};

enum uavcan_node_mode {UAVCAN_NODE_MODE_OPERATIONAL,
  UAVCAN_NODE_MODE_INITIALIZATION,
  UAVCAN_NODE_MODE_MAINTENANCE,
  UAVCAN_NODE_MODE_SOFTWAREUPDATE
};

#define UAVCAN_GET_NODE_INFO_DATA_TYPE_ID                           1
#define UAVCAN_GET_NODE_INFO_DATA_TYPE_SIGNATURE                    0xee468a8121c46a9e
#define UAVCAN_GET_NODE_INFO_RESPONSE_MAX_SIZE                      ((3015 + 7) / 8)

#define HOMEAUTOMATION_ENVIRONMENT_DATA_TYPE_ID                     20000UL
#define HOMEAUTOMATION_ENVIRONMENT_DATA_TYPE_SIGNATURE              0x0d5aa9c5110d3749
#define HOMEAUTOMATION_ENVIRONMENT_MESSAGE_SIZE                     ((56+7)/8)

#define HOMEAUTOMATION_CONDUCTION_SENSOR_DATA_TYPE_ID               20001UL
#define HOMEAUTOMATION_CONDUCTION_SENSOR_DATA_TYPE_SIGNATURE        0x38439da685c5e50d
#define HOMEAUTOMATION_CONDUCTION_SENSOR_MESSAGE_SIZE               ((67+7)/8)

#define HOMEAUTOMATION_EVENTCOUNT_DATA_TYPE_ID                      20003UL
#define HOMEAUTOMATION_EVENTCOUNT_DATA_TYPE_SIGNATURE               0x1a4b5dd9fe6f903a
#define HOMEAUTOMATION_EVENTCOUNT_MESSAGE_SIZE                      ((227+7)/8)

#define HOMEAUTOMATION_METER_DATA_TYPE_ID                           20004UL
#define HOMEAUTOMATION_METER_DATA_TYPE_SIGNATURE                    0xde1a37ed5fdc0c09
#define HOMEAUTOMATION_METER_DATA_MESSAGE_SIZE                      ((64+7)/8)

#define HOMEAUTOMATION_OBIS_DATA_TYPE_ID                            20005UL
#define HOMEAUTOMATION_OBIS_DATA_TYPE_SIGNATURE                     0x27cc893032270a92
#define HOMEAUTOMATION_OBIS_DATA_MESSAGE_SIZE                       ((96+7)/8)

#define HOMEAUTOMATION_TIME_DATA_TYPE_ID                            20006UL
#define HOMEAUTOMATION_TIME_DATA_TYPE_SIGNATURE                     0x50a53ca677dc1c85
#define HOMEAUTOMATION_TIME_MESSAGE_SIZE                            ((57+7)/8)

#define HOMEAUTOMATION_GREYWATER_PUMP_STATUS_DATA_TYPE_ID           20007UL
#define HOMEAUTOMATION_GREYWATER_PUMP_STATUS_DATA_TYPE_SIGNATURE    0xf716d32b2a6e8d44
#define HOMEAUTOMATION_GREYWATER_PUMP_STATUS_MESSAGE_SIZE           ((8+7)/8)

#define UAVCAN_BEGIN_FIRMWARE_UPDATE_DATA_TYPE_ID                   40
#define UAVCAN_BEGIN_FIRMWARE_UPDATE_DATA_TYPE_SIGNATURE            0xb7d725df72724126
#define UAVCAN_BEGIN_FIRMWARE_UPDATE_RESPONSE_MAX_SIZE              ((1031+7)/8)
#define UAVCAN_BEGIN_FIRMWARE_UPDATE_REQUEST_MAX_SIZE               ((1616+7)/8)

#define UAVCAN_FILE_READ_DATA_TYPE_ID                               48
#define UAVCAN_FILE_READ_DATA_TYPE_SIGNATURE                        0x8dcdca939f33f678
#define UAVCAN_FILE_READ_REQUEST_MAX_SIZE                           ((1648+7)/8)
#define UAVCAN_FILE_READ_RESPONSE_MAX_SIZE                          ((2073+7)/8)

#define UAVCAN_RESTART_NODE_DATA_TYPE_ID                            5
#define UAVCAN_RESTART_NODE_DATA_TYPE_SIGNATURE                     0x569e05394a3017f0
#define UAVCAN_RESTART_NODE_REQUEST_MAX_SIZE                        ((40+7)/8)
#define UAVCAN_RESTART_NODE_RESPONSE_MAX_SIZE                       ((1+7)/8)

#define UAVCAN_PARAM_GETSET_REQUEST_MAX_SIZE                        ((1791+7)/8)
#define UAVCAN_PARAM_GETSET_RESPONSE_MAX_SIZE                       ((2967+7)/8)
#define UAVCAN_PARAM_GETSET_DATA_TYPE_SIGNATURE                     0xa7b622f939d1a4d5
#define UAVCAN_PARAM_GETSET_DATA_TYPE_ID                            11

#define UAVCAN_DEBUG_LOG_MESSAGE_DATA_TYPE_ID                       16383
#define UAVCAN_DEBUG_LOG_MESSAGE_DATA_TYPE_SIGNATURE                0xd654a48e0c049d75
#define UAVCAN_DEBUG_LOG_MESSAGE_MESSAGE_SIZE                       ((983+7)/8)

#define UNIQUE_ID_LENGTH_BYTES                                      12

#endif /* UAVCAN_H_ */

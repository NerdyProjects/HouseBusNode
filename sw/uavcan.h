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

#define UAVCAN_NODE_HEALTH_OK                                       0
#define UAVCAN_NODE_HEALTH_WARNING                                  1
#define UAVCAN_NODE_HEALTH_ERROR                                    2
#define UAVCAN_NODE_HEALTH_CRITICAL                                 3

#define UAVCAN_NODE_MODE_OPERATIONAL                                0
#define UAVCAN_NODE_MODE_INITIALIZATION                             1

#define UAVCAN_GET_NODE_INFO_RESPONSE_MAX_SIZE                      ((3015 + 7) / 8)
#define UAVCAN_GET_NODE_INFO_DATA_TYPE_SIGNATURE                    0xee468a8121c46a9e
#define UAVCAN_GET_NODE_INFO_DATA_TYPE_ID                           1

#define HOMEAUTOMATION_ENVIRONMENT_MESSAGE_SIZE                     ((53+7)/8)
#define HOMEAUTOMATION_ENVIRONMENT_DATA_TYPE_ID                     20000UL
#define HOMEAUTOMATION_ENVIRONMENT_DATA_TYPE_SIGNATURE              0x5a95451888a88279


#define UNIQUE_ID_LENGTH_BYTES                                      12

#endif /* UAVCAN_H_ */

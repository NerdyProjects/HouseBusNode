/*
 * UAVCAN data structure definition for libcanard.
 *
 * Autogenerated, do not edit.
 *
 * Source file: /home/matthias/Projekte/heizungssteuerung/dsdl/homeautomation/20001.ConductionSensor.uavcan
 */

#ifndef __HOMEAUTOMATION_CONDUCTIONSENSOR
#define __HOMEAUTOMATION_CONDUCTIONSENSOR

#include <stdint.h>
#include "canard.h"

#ifdef __cplusplus
extern "C"
{
#endif

/******************************* Source text **********************************
bool error
bool[7] status
uint8[<=7] quality
******************************************************************************/

/********************* DSDL signature source definition ***********************
homeautomation.ConductionSensor
saturated bool error
saturated bool[7] status
saturated uint8[<=7] quality
******************************************************************************/

#define HOMEAUTOMATION_CONDUCTIONSENSOR_ID                 20001
#define HOMEAUTOMATION_CONDUCTIONSENSOR_NAME               "homeautomation.ConductionSensor"
#define HOMEAUTOMATION_CONDUCTIONSENSOR_SIGNATURE          (0x38439DA685C5E50DULL)

#define HOMEAUTOMATION_CONDUCTIONSENSOR_MAX_SIZE           ((67 + 7)/8)

// Constants

#define HOMEAUTOMATION_CONDUCTIONSENSOR_STATUS_LENGTH                                    7
#define HOMEAUTOMATION_CONDUCTIONSENSOR_QUALITY_MAX_LENGTH                               7

typedef struct
{
    // FieldTypes
    bool       error;                         // bit len 1
    bool       status[7];                     // Static Array 1bit[7] max items
    struct
    {
        uint8_t    len;                       // Dynamic array length
        uint8_t*   data;                      // Dynamic Array 8bit[7] max items
    } quality;

} homeautomation_ConductionSensor;

extern
uint32_t homeautomation_ConductionSensor_encode(homeautomation_ConductionSensor* source, void* msg_buf);

extern
int32_t homeautomation_ConductionSensor_decode(const CanardRxTransfer* transfer, uint16_t payload_len, homeautomation_ConductionSensor* dest, uint8_t** dyn_arr_buf);

extern
uint32_t homeautomation_ConductionSensor_encode_internal(homeautomation_ConductionSensor* source, void* msg_buf, uint32_t offset, uint8_t root_item);

extern
int32_t homeautomation_ConductionSensor_decode_internal(const CanardRxTransfer* transfer, uint16_t payload_len, homeautomation_ConductionSensor* dest, uint8_t** dyn_arr_buf, int32_t offset, uint8_t tao);

#ifdef __cplusplus
} // extern "C"
#endif
#endif // __HOMEAUTOMATION_CONDUCTIONSENSOR
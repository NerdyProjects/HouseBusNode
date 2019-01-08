/*
 * UAVCAN data structure definition for libcanard.
 *
 * Autogenerated, do not edit.
 *
 * Source file: /home/matthias/Projekte/heizungssteuerung/dsdl/homeautomation/20004.Meter.uavcan
 */

#ifndef __HOMEAUTOMATION_METER
#define __HOMEAUTOMATION_METER

#include <stdint.h>
#include "canard.h"

#ifdef __cplusplus
extern "C"
{
#endif

/******************************* Source text **********************************
uint64 count
******************************************************************************/

/********************* DSDL signature source definition ***********************
homeautomation.Meter
saturated uint64 count
******************************************************************************/

#define HOMEAUTOMATION_METER_ID                            20004
#define HOMEAUTOMATION_METER_NAME                          "homeautomation.Meter"
#define HOMEAUTOMATION_METER_SIGNATURE                     (0xDE1A37ED5FDC0C09ULL)

#define HOMEAUTOMATION_METER_MAX_SIZE                      ((64 + 7)/8)

// Constants

typedef struct
{
    // FieldTypes
    uint64_t   count;                         // bit len 64

} homeautomation_Meter;

extern
uint32_t homeautomation_Meter_encode(homeautomation_Meter* source, void* msg_buf);

extern
int32_t homeautomation_Meter_decode(const CanardRxTransfer* transfer, uint16_t payload_len, homeautomation_Meter* dest, uint8_t** dyn_arr_buf);

extern
uint32_t homeautomation_Meter_encode_internal(homeautomation_Meter* source, void* msg_buf, uint32_t offset, uint8_t root_item);

extern
int32_t homeautomation_Meter_decode_internal(const CanardRxTransfer* transfer, uint16_t payload_len, homeautomation_Meter* dest, uint8_t** dyn_arr_buf, int32_t offset, uint8_t tao);

#ifdef __cplusplus
} // extern "C"
#endif
#endif // __HOMEAUTOMATION_METER
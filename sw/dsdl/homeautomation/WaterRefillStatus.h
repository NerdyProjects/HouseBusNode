/*
 * UAVCAN data structure definition for libcanard.
 *
 * Autogenerated, do not edit.
 *
 */

#ifndef __HOMEAUTOMATION_WATERREFILLSTATUS
#define __HOMEAUTOMATION_WATERREFILLSTATUS

#include <stdint.h>
#include "canard.h"

#ifdef __cplusplus
extern "C"
{
#endif

/******************************* Source text **********************************
uint8 state
uint32 stopped_for
uint16 running_for
******************************************************************************/

/********************* DSDL signature source definition ***********************
homeautomation.WaterRefillStatus
saturated uint8 state
saturated uint32 stopped_for
saturated uint16 running_for
******************************************************************************/

#define HOMEAUTOMATION_WATERREFILLSTATUS_ID                20002
#define HOMEAUTOMATION_WATERREFILLSTATUS_NAME              "homeautomation.WaterRefillStatus"
#define HOMEAUTOMATION_WATERREFILLSTATUS_SIGNATURE         (0xEB27367F6A6BFF88ULL)

#define HOMEAUTOMATION_WATERREFILLSTATUS_MAX_SIZE          ((56 + 7)/8)

// Constants

typedef struct
{
    // FieldTypes
    uint8_t    state;                         // bit len 8
    uint32_t   stopped_for;                   // bit len 32
    uint16_t   running_for;                   // bit len 16

} homeautomation_WaterRefillStatus;

extern
uint32_t homeautomation_WaterRefillStatus_encode(homeautomation_WaterRefillStatus* source, void* msg_buf);

extern
int32_t homeautomation_WaterRefillStatus_decode(const CanardRxTransfer* transfer, uint16_t payload_len, homeautomation_WaterRefillStatus* dest, uint8_t** dyn_arr_buf);

extern
uint32_t homeautomation_WaterRefillStatus_encode_internal(homeautomation_WaterRefillStatus* source, void* msg_buf, uint32_t offset, uint8_t root_item);

extern
int32_t homeautomation_WaterRefillStatus_decode_internal(const CanardRxTransfer* transfer, uint16_t payload_len, homeautomation_WaterRefillStatus* dest, uint8_t** dyn_arr_buf, int32_t offset, uint8_t tao);

#ifdef __cplusplus
} // extern "C"
#endif
#endif // __HOMEAUTOMATION_WATERREFILLSTATUS
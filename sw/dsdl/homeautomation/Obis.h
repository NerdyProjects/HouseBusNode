/*
 * UAVCAN data structure definition for libcanard.
 *
 * Autogenerated, do not edit.
 *
 */

#ifndef __HOMEAUTOMATION_OBIS
#define __HOMEAUTOMATION_OBIS

#include <stdint.h>
#include "canard.h"

#ifdef __cplusplus
extern "C"
{
#endif

/******************************* Source text **********************************
uint8[3] code
uint8 unit
int64 value
******************************************************************************/

/********************* DSDL signature source definition ***********************
homeautomation.Obis
saturated uint8[3] code
saturated uint8 unit
saturated int64 value
******************************************************************************/

#define HOMEAUTOMATION_OBIS_ID                             20005
#define HOMEAUTOMATION_OBIS_NAME                           "homeautomation.Obis"
#define HOMEAUTOMATION_OBIS_SIGNATURE                      (0x27CC893032270A92ULL)

#define HOMEAUTOMATION_OBIS_MAX_SIZE                       ((96 + 7)/8)

// Constants

#define HOMEAUTOMATION_OBIS_CODE_LENGTH                                                  3

typedef struct
{
    // FieldTypes
    uint8_t    code[3];                       // Static Array 8bit[3] max items
    uint8_t    unit;                          // bit len 8
    int64_t    value;                         // bit len 64

} homeautomation_Obis;

extern
uint32_t homeautomation_Obis_encode(homeautomation_Obis* source, void* msg_buf);

extern
int32_t homeautomation_Obis_decode(const CanardRxTransfer* transfer, uint16_t payload_len, homeautomation_Obis* dest, uint8_t** dyn_arr_buf);

extern
uint32_t homeautomation_Obis_encode_internal(homeautomation_Obis* source, void* msg_buf, uint32_t offset, uint8_t root_item);

extern
int32_t homeautomation_Obis_decode_internal(const CanardRxTransfer* transfer, uint16_t payload_len, homeautomation_Obis* dest, uint8_t** dyn_arr_buf, int32_t offset, uint8_t tao);

#ifdef __cplusplus
} // extern "C"
#endif
#endif // __HOMEAUTOMATION_OBIS
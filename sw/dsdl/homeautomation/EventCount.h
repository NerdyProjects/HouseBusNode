/*
 * UAVCAN data structure definition for libcanard.
 *
 * Autogenerated, do not edit.
 *
 */

#ifndef __HOMEAUTOMATION_EVENTCOUNT
#define __HOMEAUTOMATION_EVENTCOUNT

#include <stdint.h>
#include "canard.h"

#ifdef __cplusplus
extern "C"
{
#endif

/******************************* Source text **********************************
uint32[<=7] events
******************************************************************************/

/********************* DSDL signature source definition ***********************
homeautomation.EventCount
saturated uint32[<=7] events
******************************************************************************/

#define HOMEAUTOMATION_EVENTCOUNT_ID                       20003
#define HOMEAUTOMATION_EVENTCOUNT_NAME                     "homeautomation.EventCount"
#define HOMEAUTOMATION_EVENTCOUNT_SIGNATURE                (0x1A4B5DD9FE6F903AULL)

#define HOMEAUTOMATION_EVENTCOUNT_MAX_SIZE                 ((227 + 7)/8)

// Constants

#define HOMEAUTOMATION_EVENTCOUNT_EVENTS_MAX_LENGTH                                      7

typedef struct
{
    // FieldTypes
    struct
    {
        uint8_t    len;                       // Dynamic array length
        uint32_t*  data;                      // Dynamic Array 32bit[7] max items
    } events;

} homeautomation_EventCount;

extern
uint32_t homeautomation_EventCount_encode(homeautomation_EventCount* source, void* msg_buf);

extern
int32_t homeautomation_EventCount_decode(const CanardRxTransfer* transfer, uint16_t payload_len, homeautomation_EventCount* dest, uint8_t** dyn_arr_buf);

extern
uint32_t homeautomation_EventCount_encode_internal(homeautomation_EventCount* source, void* msg_buf, uint32_t offset, uint8_t root_item);

extern
int32_t homeautomation_EventCount_decode_internal(const CanardRxTransfer* transfer, uint16_t payload_len, homeautomation_EventCount* dest, uint8_t** dyn_arr_buf, int32_t offset, uint8_t tao);

#ifdef __cplusplus
} // extern "C"
#endif
#endif // __HOMEAUTOMATION_EVENTCOUNT
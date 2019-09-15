/*
 * UAVCAN data structure definition for libcanard.
 *
 * Autogenerated, do not edit.
 *
 * Source file: /home/matthias/Projekte/heizungssteuerung/dsdl/homeautomation/20010.BoilerStatus.uavcan
 */

#ifndef __HOMEAUTOMATION_BOILERSTATUS
#define __HOMEAUTOMATION_BOILERSTATUS

#include <stdint.h>
#include "canard.h"

#ifdef __cplusplus
extern "C"
{
#endif

/******************************* Source text **********************************
uint8 duty_cycle
int19 temperature
uint3 priority
******************************************************************************/

/********************* DSDL signature source definition ***********************
homeautomation.BoilerStatus
saturated uint8 duty_cycle
saturated int19 temperature
saturated uint3 priority
******************************************************************************/

#define HOMEAUTOMATION_BOILERSTATUS_ID                     20010
#define HOMEAUTOMATION_BOILERSTATUS_NAME                   "homeautomation.BoilerStatus"
#define HOMEAUTOMATION_BOILERSTATUS_SIGNATURE              (0x5089C50337849612ULL)

#define HOMEAUTOMATION_BOILERSTATUS_MAX_SIZE               ((30 + 7)/8)

// Constants

typedef struct
{
    // FieldTypes
    uint8_t    duty_cycle;                    // bit len 8
    int32_t    temperature;                   // bit len 19
    uint8_t    priority;                      // bit len 3

} homeautomation_BoilerStatus;

extern
uint32_t homeautomation_BoilerStatus_encode(homeautomation_BoilerStatus* source, void* msg_buf);

extern
int32_t homeautomation_BoilerStatus_decode(const CanardRxTransfer* transfer, uint16_t payload_len, homeautomation_BoilerStatus* dest, uint8_t** dyn_arr_buf);

extern
uint32_t homeautomation_BoilerStatus_encode_internal(homeautomation_BoilerStatus* source, void* msg_buf, uint32_t offset, uint8_t root_item);

extern
int32_t homeautomation_BoilerStatus_decode_internal(const CanardRxTransfer* transfer, uint16_t payload_len, homeautomation_BoilerStatus* dest, uint8_t** dyn_arr_buf, int32_t offset, uint8_t tao);

#ifdef __cplusplus
} // extern "C"
#endif
#endif // __HOMEAUTOMATION_BOILERSTATUS
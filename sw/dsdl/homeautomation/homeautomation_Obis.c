/*
 * UAVCAN data structure definition for libcanard.
 *
 * Autogenerated, do not edit.
 *
 */
#include "homeautomation/Obis.h"
#include "canard.h"

#ifndef CANARD_INTERNAL_SATURATE
#define CANARD_INTERNAL_SATURATE(x, max) ( ((x) > max) ? max : ( (-(x) > max) ? (-max) : (x) ) );
#endif

#ifndef CANARD_INTERNAL_SATURATE_UNSIGNED
#define CANARD_INTERNAL_SATURATE_UNSIGNED(x, max) ( ((x) > max) ? max : (x) );
#endif

#define CANARD_INTERNAL_ENABLE_TAO  ((uint8_t) 1)
#define CANARD_INTERNAL_DISABLE_TAO ((uint8_t) 0)

#if defined(__GNUC__)
# define CANARD_MAYBE_UNUSED(x) x __attribute__((unused))
#else
# define CANARD_MAYBE_UNUSED(x) x
#endif

/**
  * @brief homeautomation_Obis_encode_internal
  * @param source : pointer to source data struct
  * @param msg_buf: pointer to msg storage
  * @param offset: bit offset to msg storage
  * @param root_item: for detecting if TAO should be used
  * @retval returns offset
  */
uint32_t homeautomation_Obis_encode_internal(homeautomation_Obis* source,
  void* msg_buf,
  uint32_t offset,
  uint8_t CANARD_MAYBE_UNUSED(root_item))
{
    uint32_t c = 0;

    // Static array (code)
    for (c = 0; c < 3; c++)
    {
        canardEncodeScalar(msg_buf, offset, 8, (void*)(source->code + c)); // 255
        offset += 8;
    }

    canardEncodeScalar(msg_buf, offset, 8, (void*)&source->unit); // 255
    offset += 8;

    canardEncodeScalar(msg_buf, offset, 64, (void*)&source->value); // 9223372036854775807
    offset += 64;

    return offset;
}

/**
  * @brief homeautomation_Obis_encode
  * @param source : Pointer to source data struct
  * @param msg_buf: Pointer to msg storage
  * @retval returns message length as bytes
  */
uint32_t homeautomation_Obis_encode(homeautomation_Obis* source, void* msg_buf)
{
    uint32_t offset = 0;

    offset = homeautomation_Obis_encode_internal(source, msg_buf, offset, 1);

    return (offset + 7 ) / 8;
}

/**
  * @brief homeautomation_Obis_decode_internal
  * @param transfer: Pointer to CanardRxTransfer transfer
  * @param payload_len: Payload message length
  * @param dest: Pointer to destination struct
  * @param dyn_arr_buf: NULL or Pointer to memory storage to be used for dynamic arrays
  *                     homeautomation_Obis dyn memory will point to dyn_arr_buf memory.
  *                     NULL will ignore dynamic arrays decoding.
  * @param offset: Call with 0, bit offset to msg storage
  * @param tao: is tail array optimization used
  * @retval offset or ERROR value if < 0
  */
int32_t homeautomation_Obis_decode_internal(
  const CanardRxTransfer* transfer,
  uint16_t CANARD_MAYBE_UNUSED(payload_len),
  homeautomation_Obis* dest,
  uint8_t** CANARD_MAYBE_UNUSED(dyn_arr_buf),
  int32_t offset,
  uint8_t CANARD_MAYBE_UNUSED(tao))
{
    int32_t ret = 0;
    uint32_t c = 0;

    // Static array (code)
    for (c = 0; c < 3; c++)
    {
        ret = canardDecodeScalar(transfer, offset, 8, false, (void*)(dest->code + c));
        if (ret != 8)
        {
            goto homeautomation_Obis_error_exit;
        }
        offset += 8;
    }

    ret = canardDecodeScalar(transfer, offset, 8, false, (void*)&dest->unit);
    if (ret != 8)
    {
        goto homeautomation_Obis_error_exit;
    }
    offset += 8;

    ret = canardDecodeScalar(transfer, offset, 64, true, (void*)&dest->value);
    if (ret != 64)
    {
        goto homeautomation_Obis_error_exit;
    }
    offset += 64;
    return offset;

homeautomation_Obis_error_exit:
    if (ret < 0)
    {
        return ret;
    }
    else
    {
        return -CANARD_ERROR_INTERNAL;
    }
}

/**
  * @brief homeautomation_Obis_decode
  * @param transfer: Pointer to CanardRxTransfer transfer
  * @param payload_len: Payload message length
  * @param dest: Pointer to destination struct
  * @param dyn_arr_buf: NULL or Pointer to memory storage to be used for dynamic arrays
  *                     homeautomation_Obis dyn memory will point to dyn_arr_buf memory.
  *                     NULL will ignore dynamic arrays decoding.
  * @retval offset or ERROR value if < 0
  */
int32_t homeautomation_Obis_decode(const CanardRxTransfer* transfer,
  uint16_t payload_len,
  homeautomation_Obis* dest,
  uint8_t** dyn_arr_buf)
{
    const int32_t offset = 0;
    int32_t ret = 0;

    /* Backward compatibility support for removing TAO
     *  - first try to decode with TAO DISABLED
     *  - if it fails fall back to TAO ENABLED
     */
    uint8_t tao = CANARD_INTERNAL_DISABLE_TAO;

    while (1)
    {
        // Clear the destination struct
        for (uint32_t c = 0; c < sizeof(homeautomation_Obis); c++)
        {
            ((uint8_t*)dest)[c] = 0x00;
        }

        ret = homeautomation_Obis_decode_internal(transfer, payload_len, dest, dyn_arr_buf, offset, tao);

        if (ret >= 0)
        {
            break;
        }

        if (tao == CANARD_INTERNAL_ENABLE_TAO)
        {
            break;
        }
        tao = CANARD_INTERNAL_ENABLE_TAO;
    }

    return ret;
}

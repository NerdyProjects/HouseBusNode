/*
 * dewpoint.c
 *
 *  Created on: 01.12.2018
 *      Author: matthias
 */

#include "stdint.h"
#include "qfplib.h"

/* calculates the dew point for given temperature and relative humidity.
 * Temperature is given in centi degrees celsius (100 equals 1.00 degrees)
 * relative humidity is given in milli percent (1000 equals 1.000 %)
 */
int16_t calculateDewPoint(int32_t centiTemperature, uint32_t milliHumidity) {
  float temp = qfp_fdiv(qfp_int2float(centiTemperature), 100);
  float rh = qfp_fdiv(qfp_int2float(milliHumidity), 100000);
  float a = 7.5f;
  float b = 237.3f;
  float acc = qfp_fdiv(qfp_fmul(a, temp), qfp_fadd(b, temp));
  float sdd = qfp_fexp(qfp_fmul(acc, 2.30258509299f)); /* sdd excludes factor 6.1078 */
  float dd = qfp_fmul(rh, sdd); /* dd excludes factor 6.1078 from sdd */
  float v = qfp_fdiv(qfp_fln(dd), 2.30258509299f);
  float res = qfp_fdiv(qfp_fmul(b, v), qfp_fsub(a, v));

  return qfp_float2int(qfp_fmul(res, 100));
}

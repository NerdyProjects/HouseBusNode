/*
 * dewpoint.c
 *
 *  Created on: 01.12.2018
 *      Author: matthias
 */

#include "stdint.h"
#include "qfplib.h"
#include "modules/bme280.h"
#include "modules/temperature_receiver.h"
#include "dewpoint.h"
#include "ch.h"

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

/* although it might be stupid, a long interval seems reasonable to work around broken nodes */
#define OUTSIDE_TEMPERATURE_MAX_AGE_S 86400

/**
 * approximates a wall temperature from the first available local temperature sensor and a remote temperature sensor.
 *
 * @param wallDewPointTemperatureFactor influences the wall temperature calculation: t = outside + (inside - outside) * factor / 1024
 *
 * Returns the difference of the wall temperature to the dew point of the local air, in centi degrees celsius, e.g.
 * 500 when the wall is 5 degrees warmer than the dew point, in the parameter out.
 *
 * Returns 0 on success, negative on error.
 */
int approximateWallTemperatureDifferenceToDewPoint(int16_t *out, uint16_t wallTemperatureFactor) {
  if(bme_presence)
  {
    sysinterval_t age;
    int16_t outsideTemperature;
    int16_t insideDewPoint;
    int16_t insideTemperature;
    uint32_t insideHumidity;
    /* Todo: We want to work with average (12-24h) outside temperature */
    outsideTemperature = getTargetTemperature(&age);
    if(bme_presence & 1)
    {
      insideTemperature = BMECentiTemperature[0];
      insideHumidity = BMEMilliHumidity[0];
    } else {
      insideTemperature = BMECentiTemperature[1];
      insideHumidity = BMEMilliHumidity[1];
    }
    if(age < TIME_S2I(OUTSIDE_TEMPERATURE_MAX_AGE_S))
    {
      /* both temperature readings available */
      int16_t wallTemperature;

      /* a factor defines the ratio between inside and outside temperature roughly equalling thermal flow / wall isolation */
      wallTemperature = outsideTemperature + (((int32_t)(insideTemperature - outsideTemperature)) * wallTemperatureFactor / 1024);
      insideDewPoint = calculateDewPoint(insideTemperature, insideHumidity);
      *out = wallTemperature - insideDewPoint;
      return 0;
    }
  }
  return -1;
}


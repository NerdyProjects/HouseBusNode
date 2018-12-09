/*
 * dewpoint.h
 *
 *  Created on: 01.12.2018
 *      Author: matthias
 */

#ifndef MODULES_DEWPOINT_H_
#define MODULES_DEWPOINT_H_

int16_t calculateDewPoint(int32_t centiTemperature, uint32_t milliHumidity);
int approximateWallTemperatureDifferenceToDewPoint(int16_t *out, uint16_t wallTemperatureFactor);

#endif /* MODULES_DEWPOINT_H_ */

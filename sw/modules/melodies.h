/*
 * melodies.h
 *
 *  Created on: 01.12.2018
 *      Author: matthias
 */

#ifndef MODULES_MELODIES_H_
#define MODULES_MELODIES_H_

#include "stdint.h"

typedef struct
{
  uint8_t note;
  uint8_t duration;
} note;

typedef note* melody;


#endif /* MODULES_MELODIES_H_ */

/*
 * config.h
 *
 *  Created on: 24.01.2018
 *      Author: matthias
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdint.h>

struct nodeconfig {
  uint8_t node_id;
  uint8_t node_name[80];
};

extern struct nodeconfig nodeconfig;
#endif /* CONFIG_H_ */

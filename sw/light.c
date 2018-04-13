/*
 * light.c
 *
 *  Created on: 13.04.2018
 *      Author: matthias
 */


#include "config.h"
#include "drivers/pwm.h"

void light_init(void)
{
  if(config_get_uint(CONFIG_HAS_LIGHT_CONTROL))
  {
    pwm_init();
    for(int i = 1; i <= 8; ++i)
    {
      pwm_set_dc(i, 0xFF + 100*i);
    }
  }
}

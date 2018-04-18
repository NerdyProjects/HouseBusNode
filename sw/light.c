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
    /* Hallway light beginning from door .. backyard */
    for(int i = 1; i <= 5; ++i)
    {
      pwm_set_dc(i, 150);
    }
    /* K20-1 staircase light */
    pwm_set_dc(6, 1500);
    /* K20-2 staircase light */
    pwm_set_dc(7, 40);

    /* GPIOB 3 is motion sensor input, active high */
  }
}

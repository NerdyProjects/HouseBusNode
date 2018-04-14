/* Module for controlling up to 8 PWM channels for general purposes. */

/* Pinout:
 * PWM for open drain switches (high pulls output low -> enables light)
 * PA2 (T15 CH1, T2 CH3)
 * PA3 (T15 CH2, T2 CH4)
 * PA4 (T14 CH1)
 * PA6 (T16 CH1, T3 CH1)
 * PA7 (T14 CH1, T17 CH1, T1 CH1N, T3 CH2)
 * PB0 (T1 CH2N, T3 CH3)
 * PB1 (T14 CH1, T3 CH4)
 * PA8 (T1 CH1)
 *
 * -> T1 CH1, T3 CH1-4, T14 CH1, T15 CH1-2
 */

#include "hal.h"

void pwm_init(void)
{

  /* The ChibiOs timer configuration is not really suited to our use case as it uses a fixed set of timers that does not match ours.
   * Initialize manually
   */
  rccEnableTIM1(FALSE);
  rccResetTIM1();
  rccEnableTIM3(FALSE);
  rccResetTIM3();
  rccEnableTIM14(FALSE);
  rccResetTIM14();
  rccEnableTIM15(FALSE);
  rccResetTIM15();

  STM32_TIM1->CCMR1 = STM32_TIM_CCMR1_OC1M(6) | STM32_TIM_CCMR1_OC1PE;

  STM32_TIM3->CCMR1 = STM32_TIM_CCMR1_OC1M(6) | STM32_TIM_CCMR1_OC1PE |
                     STM32_TIM_CCMR1_OC2M(6) | STM32_TIM_CCMR1_OC2PE;
  STM32_TIM3->CCMR2 = STM32_TIM_CCMR2_OC3M(6) | STM32_TIM_CCMR2_OC3PE |
                     STM32_TIM_CCMR2_OC4M(6) | STM32_TIM_CCMR2_OC4PE;

  STM32_TIM14->CCMR1 = STM32_TIM_CCMR1_OC1M(6) | STM32_TIM_CCMR1_OC1PE;

  STM32_TIM15->CCMR1 = STM32_TIM_CCMR1_OC1M(6) | STM32_TIM_CCMR1_OC1PE |
                     STM32_TIM_CCMR1_OC2M(6) | STM32_TIM_CCMR1_OC2PE;

  /* TODO: APB Clock is assumed 48 MHz fixed, that gives us ~732 Hz PWM frequency without prescaler. Seems good :-) */
  STM32_TIM1->CCER = STM32_TIM_CCER_CC1E;
  STM32_TIM3->CCER = STM32_TIM_CCER_CC1E | STM32_TIM_CCER_CC2E | STM32_TIM_CCER_CC3E | STM32_TIM_CCER_CC4E;
  STM32_TIM14->CCER = STM32_TIM_CCER_CC1E;
  STM32_TIM15->CCER = STM32_TIM_CCER_CC1E | STM32_TIM_CCER_CC2E;

  STM32_TIM1->EGR   = STM32_TIM_EGR_UG;
  STM32_TIM1->SR    = 0;
  STM32_TIM3->EGR   = STM32_TIM_EGR_UG;
  STM32_TIM3->SR    = 0;
  STM32_TIM14->EGR   = STM32_TIM_EGR_UG;
  STM32_TIM14->SR    = 0;
  STM32_TIM15->EGR   = STM32_TIM_EGR_UG;
  STM32_TIM15->SR    = 0;

  STM32_TIM1->BDTR  = STM32_TIM_BDTR_MOE;
  STM32_TIM1->CR1   = STM32_TIM_CR1_ARPE | STM32_TIM_CR1_URS |
                      STM32_TIM_CR1_CEN;
  STM32_TIM3->BDTR  = STM32_TIM_BDTR_MOE;
  STM32_TIM3->CR1   = STM32_TIM_CR1_ARPE | STM32_TIM_CR1_URS |
                      STM32_TIM_CR1_CEN;
  STM32_TIM14->BDTR  = STM32_TIM_BDTR_MOE;
  STM32_TIM14->CR1   = STM32_TIM_CR1_ARPE | STM32_TIM_CR1_URS |
                      STM32_TIM_CR1_CEN;
  STM32_TIM15->BDTR  = STM32_TIM_BDTR_MOE;
  STM32_TIM15->CR1   = STM32_TIM_CR1_ARPE | STM32_TIM_CR1_URS |
                      STM32_TIM_CR1_CEN;
  palSetPadMode(GPIOA, 2, PAL_MODE_ALTERNATE(0));
  palSetPadMode(GPIOA, 3, PAL_MODE_ALTERNATE(0));
  palSetPadMode(GPIOA, 4, PAL_MODE_ALTERNATE(4));
  palSetPadMode(GPIOA, 6, PAL_MODE_ALTERNATE(1));
  palSetPadMode(GPIOA, 7, PAL_MODE_ALTERNATE(1));
  palSetPadMode(GPIOA, 8, PAL_MODE_ALTERNATE(2));
  palSetPadMode(GPIOB, 0, PAL_MODE_ALTERNATE(1));
  palSetPadMode(GPIOB, 1, PAL_MODE_ALTERNATE(1));
}

void pwm_set_dc(uint8_t channel, uint16_t dc)
{
  switch (channel)
  {
  case 1:
    STM32_TIM15->CCR[0] = dc;
    break;
  case 2:
    STM32_TIM15->CCR[1] = dc;
    break;
  case 3:
    STM32_TIM14->CCR[0] = dc;
    break;
  case 4:
    STM32_TIM3->CCR[0] = dc;
    break;
  case 5:
    STM32_TIM3->CCR[1] = dc;
    break;
  case 6:
    STM32_TIM3->CCR[2] = dc;
    break;
  case 7:
    STM32_TIM3->CCR[3] = dc;
    break;
  case 8:
    STM32_TIM1->CCR[0] = dc;
    break;
  default:
    break;
  }
}

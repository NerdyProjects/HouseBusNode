#include "hal.h"
#include "qfplib.h"
#include "node.h"


void tone_init(void)
{
  rccEnableTIM14(FALSE);
  rccResetTIM14();
  STM32_TIM14->CCMR1 = STM32_TIM_CCMR1_OC1M(6) | STM32_TIM_CCMR1_OC1PE;

  STM32_TIM14->EGR   = STM32_TIM_EGR_UG;
  STM32_TIM14->SR    = 0;

  chThdSleep(TIME_US2I(250));
  STM32_TIM14->BDTR  = STM32_TIM_BDTR_MOE;
  STM32_TIM14->CR1   = STM32_TIM_CR1_ARPE | STM32_TIM_CR1_URS |
      STM32_TIM_CR1_CEN;
  palSetPadMode(GPIOA, 7, PAL_MODE_ALTERNATE(4));
}

void tone_output_note(uint8_t note, uint8_t volume)
{
 /*
  * d = 48000000/(2^((x-69)/12)*440)
  * => d = 48000000/(2^(x/12-69/12)*440)
  * => d = 48000000/(2^(x/12)*2^(-69/12)*440)
  * => d = 5870985.88104 / (2^(x/12))
  * => d = 5870985.88104 / e^(x*0.05776226504666)
  */
  uint32_t divider = qfp_float2uint(qfp_fdiv(5870985.88104f, qfp_fexp(qfp_fmul(qfp_int2float(note), 0.05776226504666f))));
  /*
   * timer overflow rate is 48 MHz / (prescaler + 1) / (reload + 1)
   * ex.: divider is 1..65536: reload = divider; prescaler = 0
   * for dividers > 65536, prescaler needs to be adjusted; for simplicity and as the proposed use case does not allow deep frequencies,
   * just use a prescaler of 1 (so it divides by 2) and reload = divider / 2.
   */
  uint32_t prescaler = 1;
  while(divider > 0x0FFFF)
  {
    prescaler *= 2;
    divider /= 2;
  }
  STM32_TIM14->PSC = prescaler - 1;
  STM32_TIM14->ARR = divider - 1;
  STM32_TIM14->CCER = STM32_TIM_CCER_CC1E | STM32_TIM_CCER_CC1P; // inverted
  /* volume set via duty cycle which depends on the divider/note. In expected frequency range,
   * divider will be greater than 12000.
   * Maximum volume: CCR = divider / 2
   * Zero volume: CCR = 0
   */
  if(volume == 255) {
    STM32_TIM14->CCR[0] = divider / 2;
  } else if(volume == 0) {
    STM32_TIM14->CCR[0] = 0;
  } else {
    /* this is not very accurate but good enough for big dividers */
    STM32_TIM14->CCR[0] = (divider / 8 / 256) * volume;
  }
}

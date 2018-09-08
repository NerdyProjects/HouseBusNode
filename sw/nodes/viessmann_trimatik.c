
/* Application to be used ass Viessmann Trimatik MC Elektronikbox replacement.
 * Use with Hausspielerei PCB.
 * Pinout:
 * Pullup (3.3V/Avcc) for all analog input channels via PA0 (negated)
 * outside temperature sensor (PT500 with 500 Ohms pullup), filtered by 10k series + 330 nF, PA1 (AIN1)
 * burner temperature sensor (PT500 with 500 Ohms pullup), filtered by ~1µF, PA2 (AIN2)
 * flow temperature sensor (PT500 with 500 Ohms pullup), filtered by ~1µF, PA3 (AIN3)
 * (AIN4-6 for STS/FB1/FB2 currently NC/unused)
 * LED A (+red on PCB), PA7
 * LED B, PB0
 * circulation pump A, PA11
 * circulation pump B, green LED on PCB, PA15
 * burner on (has external ignition and safety control, ignition takes nearly a minute), PA12
 * sense input (unknown - maybe burner feedback or override switch), PB11
 * Key matrices:
 * out sw 1/2: PB1
 * out sw 3/4: PB2
 * out sw 5/6: PB10
 * in sw B0 2/4/6: PC14
 * in sw B1 2/4/6: PC15
 * in sw B2 2/4/6: PF0
 * in sw B3 2/4/6: PF1
 * in sw B0 1/3/5: PB14
 * in sw B1 1/3/5: PA8
 * in sw B2 1/3/5: PA10
 * in sw B3 1/3/5: PB3
 *
 */

#ifdef NODE_VIESSMANN_TRIMATIK
#include "hal.h"
#include "qfplib.h"
#include "../drivers/analog.h"
#include "../node.h"


typedef struct {
  float c0;
  float c1;
  float c2;
} correction_coefficient_t;

enum
{
  KEY_MODE = 1
};

enum
{
  KEY_MODE_DEBUG = 6
};

static correction_coefficient_t pt500[3];
static int16_t temperature[3];
/* keys:
 * 0 2 4
 * 1 3 5
 *
 * each from 0-15, where labels are as follows:
 * 0 (Sun): -7 .. +8 (0 is N)
 * 2 (Moon): -9 .. +6 (0 is N)
 * 4 (Water): 0, 32 .. 50 (each step is +2)
 * 1 (Mode): 0 Freeze Freeze, 1 Sun Freeze, 2 Sun Moon, 3 Sun Sun, 4 Moon Moon, 5 Water Freeze, 6..15: 1..10
 * 3 (slope): 0,2 .. 2,6 (each step is +0,2)
 * 5 (offset): -12 .. +33 (each step is +3)
 */
static uint8_t key[6];
static uint8_t key_next[6];


/* slowest filter needs ~30 ms to settle, that is 6 5ms ticks.
 * add one each for synchronisation and AD conversion
 * and 5 for digital filter reset/settling */
#define ANALOG_FILTER_TICKS 13
volatile uint8_t analog_evaluate_in_ticks;

static void burner(uint8_t enable)
{
  palWritePad(GPIOA, GPIOA_K1_FIRE, enable ? PAL_LOW : PAL_HIGH);
}

static void circulation(uint8_t enable)
{
  palWritePad(GPIOA, GPIOA_K2_20A, enable ? PAL_LOW : PAL_HIGH);
}

static void analog_pullup(uint8_t enable)
{
  if(enable)
  {
    palClearPad(GPIOA, GPIOA_PU_OFF);
  }
  else
  {
    palSetPad(GPIOA, GPIOA_PU_OFF);
  }
}

static void led_a(uint8_t enable)
{
  if(enable)
  {
    palSetPad(GPIOA, GPIOA_LED_A);
  } else
  {
    palClearPad(GPIOA, GPIOA_LED_A);
  }
}

static void led_b(uint8_t enable)
{
  if(enable)
  {
    palSetPad(GPIOB, GPIOB_LED_GREEN);
  } else
  {
    palClearPad(GPIOB, GPIOB_LED_GREEN);
  }
}

/* converts adc input (aref = PT500 vcc, resistor divider with 510 Ohms), 16 bit to temperature in celsius */
static int16_t pt500_adc_to_centicelsius(uint16_t raw_adc, correction_coefficient_t *coef)
{
  /* 0 degrees: 500 ohms (32768)
   * 10 degrees: 519.510 ohms (33395)
   * 20 degrees: 538.965 ohms (33997)
   * 50 degrees: 596.975 ohms (35665)
   * 100 degrees: 692.50 ohms (38058)
   * T = c0 + c1 * u + c2 * u²
   * coefficients for 510 ohms:
   * 1.7977322744102137e+004
   * -2.6535434581358079e+000
   * 6.4705225905651158e-005
   * -> should be calibrated in application per channel
   * Calibration using room temperature and old pipe thermometer:
   * 18-19 degrees in basement and 21-22 outside:
   * 33910 33770 33935
   * 64 degrees burner/pipe:
   * 33910 35620 36420
   * (results seem really bad :( )
   */
  float u = qfp_int2float(raw_adc);
  float t = qfp_fadd(coef->c0, qfp_fadd(qfp_fmul(coef->c1, u), qfp_fmul(qfp_fmul(u, u), coef->c2)));
  return qfp_float2int(t);
}



void app_init(void)
{
  /* GPIOs are setup in board.h configuration globally (at least for now) */

  /* Key GPIOs */
  palSetPadMode(GPIOB, GPIOB_SW_C12, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOB, GPIOB_SW_C34, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOB, GPIOB_SW_C56, PAL_MODE_OUTPUT_PUSHPULL);
  palClearPort(GPIOB, PAL_PORT_BIT(GPIOB_SW_C12) | PAL_PORT_BIT(GPIOB_SW_C34) | PAL_PORT_BIT(GPIOB_SW_C56));
  palSetPadMode(GPIOA, GPIOA_SW_B1_135, PAL_MODE_INPUT_PULLDOWN);
  palSetPadMode(GPIOA, GPIOA_SW_B2_135, PAL_MODE_INPUT_PULLDOWN);
  palSetPadMode(GPIOB, GPIOB_SW_B0_135, PAL_MODE_INPUT_PULLDOWN);
  palSetPadMode(GPIOB, GPIOB_SW_B3_135, PAL_MODE_INPUT_PULLDOWN);
  palSetPadMode(GPIOC, GPIOC_SW_B0_246, PAL_MODE_INPUT_PULLDOWN);
  palSetPadMode(GPIOC, GPIOC_SW_B1_246, PAL_MODE_INPUT_PULLDOWN);
  palSetPadMode(GPIOF, GPIOF_SW_B2_246, PAL_MODE_INPUT_PULLDOWN);
  palSetPadMode(GPIOF, GPIOF_SW_B3_246, PAL_MODE_INPUT_PULLDOWN);
  palSetPad(GPIOB, GPIOB_SW_C12);
}

void debug_keys(void)
{
  char dbgbuf[20];
  chsnprintf(dbgbuf, 20, "%2u %2u %2u %2u %2u %2u", key[0], key[1], key[2], key[3], key[4], key[5]);
  node_debug(LOG_LEVEL_DEBUG, "TRIMATIK_KEYS", dbgbuf);
}

void debug_analog(void)
{
  char dbgbuf[20];
  chsnprintf(dbgbuf, 20, "%5u %5u %5u", adc_smp_filtered[1], adc_smp_filtered[2], adc_smp_filtered[3]);
  node_debug(LOG_LEVEL_DEBUG, "TRIMATIK_ADC", dbgbuf);
}

void app_fast_tick(void)
{
  {
    /* read keys */
    static uint8_t currentKey = 0;
    uint8_t inA =
        palReadPad(GPIOB, GPIOB_SW_B0_135) |
        (palReadPad(GPIOA, GPIOA_SW_B1_135) << 1) |
        (palReadPad(GPIOA, GPIOA_SW_B2_135) << 2) |
        (palReadPad(GPIOB, GPIOB_SW_B3_135) << 3);
    uint8_t inB =
        (palReadPad(GPIOC, GPIOC_SW_B0_246) << 3) |
        (palReadPad(GPIOC, GPIOC_SW_B1_246) << 2) |
        (palReadPad(GPIOF, GPIOF_SW_B2_246) << 1) |
        (palReadPad(GPIOF, GPIOF_SW_B3_246));
    if(key_next[currentKey*2] == inA)
    {
      key[currentKey*2] = inA;
    } else
    {
      key_next[currentKey*2] = inA;
    }
    if(key_next[currentKey*2+1] == inB)
    {
      key[currentKey*2+1] = inB;
    } else {
      key_next[currentKey*2+1] = inB;
    }

    switch(currentKey)
    {
    case 0:
      currentKey = 1;
      palClearPad(GPIOB, GPIOB_SW_C12);
      palSetPad(GPIOB, GPIOB_SW_C34);
      break;
    case 1:
      currentKey = 2;
      palClearPad(GPIOB, GPIOB_SW_C34);
      palSetPad(GPIOB, GPIOB_SW_C56);
      break;
    case 2:
      currentKey = 0;
      palClearPad(GPIOB, GPIOB_SW_C56);
      palSetPad(GPIOB, GPIOB_SW_C12);
      break;
    default:
      break;
    }
  }

  if(analog_evaluate_in_ticks)
  {
    analog_evaluate_in_ticks--;
    if(analog_evaluate_in_ticks == 5)
    {
      analog_filter_reset((1 << 1) | (1 << 2) | (1 << 3));
    }
    if(analog_evaluate_in_ticks == 0)
    {
      temperature[0] = pt500_adc_to_centicelsius(adc_smp_filtered[1], &pt500[0]);
      temperature[1] = pt500_adc_to_centicelsius(adc_smp_filtered[2], &pt500[1]);
      temperature[2] = pt500_adc_to_centicelsius(adc_smp_filtered[3], &pt500[2]);
      if(key[KEY_MODE] == KEY_MODE_DEBUG)
      {
        debug_analog();
      }
      analog_pullup(0);
    }
  }
}

void app_tick(void)
{
  static uint8_t led_count = 0;
  analog_pullup(1);
  analog_evaluate_in_ticks = ANALOG_FILTER_TICKS;
  debug_keys();
  led_a(led_count & 1);
  led_b(led_count & 2);
  led_count++;
  burner(key[1] == 6);
  circulation(key[3] == 1);
}
#endif

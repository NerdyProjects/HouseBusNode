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

#include "hal.h"
#include "qfplib.h"
#include "../drivers/analog.h"
#include "../node.h"
#include "../config.h"
#include "modules/time_data.h"
#include "node.h"


/* C0 + C1 * x + c2*x² */
typedef struct {
  float c0;
  float c1;
  float c2;
} correction_coefficient_t;

enum
{
  KEY_MODE = 1,
  KEY_DAY = 0,
  KEY_NIGHT = 2,
  KEY_WATER = 4,
  KEY_SLOPE = 3,
  KEY_OFFSET = 5
};

enum
{
  KEY_MODE_OFF = 0,
  KEY_MODE_DAY_OFF = 1,
  KEY_MODE_DAY_NIGHT = 2,
  KEY_MODE_DAY_DAY = 3,
  KEY_MODE_NIGHT_HIGHT = 4,
  KEY_MODE_WATER_ONLY = 5,
  KEY_MODE_DEBUG = 6,
};

/* all coefficients the same for now, calculated on
 * 33422  1430
33371  1320
33066  990
32888  610
32875  530
** flow:
32900 540
34658  2800
37240  7500
TODO check with colder outside temperatures!
 *
 */
static correction_coefficient_t pt500[3] = {
    {8.1554856353217765e+004, -6.0174207218510478e+000, 1.0817223702997432e-004},
    {8.1554856353217765e+004, -6.0174207218510478e+000, 1.0817223702997432e-004},
    {8.1554856353217765e+004, -6.0174207218510478e+000, 1.0817223702997432e-004}
};

static int16_t flowHysteresisTurnOff;
static int16_t flowHysteresisTurnOn;
/* heating mode disabled below this target temperature */
static uint16_t minFlowTempToEnable;
/* circulation stopped below this actual temperature */
static uint16_t minTempToEnableCirculation;
/* Stop burner after this time when burner temp increases over flow temp */
/* Background: Burner temperature is strongly affected by return flow temperature. In normal load situation, it
 * is always a bit below the actual flow temperature.
 */
static uint16_t burnerStopTempRiseTime;
/* When burner stops, it is not allowed to start within given timeframe */
static uint16_t burnerMinOffTime;

/* when burning temperature is considered for thresholds, how long should the circulation be running before?
 * short: What's the maximum accepted time it takes from hot water to reach the flow?
 */
static uint16_t circulationMinOnTimeForThreshold;

/*
 * deviation is calculated as the integral of deviation of flow to real temp. It is used to control an averaged flow temperature.
 * heater turns on, when deviation is less than -hysteresis.
 */
static int32_t deviationHysteresis;

/* Temp 1 is active from time1Start to time1Stop. Otherwise, Temp 2 is active.
 * Temp is seen as the "Normal" temperature, the offset from the switch selection is added.
 * Times are encoded with minutes in the least significant byte and hours in the next more significant byte.:
 * (you can calculate the time number in a calculator with hours * 256 + minutes)
 */
static uint32_t time1Start;
static uint32_t time1Stop;
static int16_t time1Temp;
static int16_t time2Temp;

/* delay of burner request to actual fire in seconds.
 * For statistics; might be used for regulation as well
 */
#define BURNER_REQUEST_DELAY 50

static int16_t temperature[3];
/* raw adc values extended to 32 bit for higher filter precision. A second slow IIR filter implemented here */
static uint32_t adc_double_filtered[3];

/* keys:
 * 0 2 4
 * 1 3 5
 *
 * each from 0-15, where labels are as follows:
 * 0 (Sun): -7 .. +8 (0 is N)
 * 2 (Moon): -9 .. +6 (0 is N)
 * 4 (Water): 0, 32 .. 50 (each step is +2)
 * 1 (Mode): 0 Freeze Freeze, 1 Sun Freeze, 2 Sun Moon, 3 Sun Sun, 4 Moon Moon, 5 Water Freeze, 6..15: 1..10
 * 3 (slope): 0,2 .. 2,6 (step size varies)
 * 5 (offset): -12 .. +33 (each step is +3)
 */
static uint8_t key[6];
static uint8_t key_next[6];


/* slowest filter needs ~30 ms to settle, that is 6 5ms ticks.
 * add one each for synchronisation and AD conversion
 * and 6 for digital filter reset/settling
 * as the results are still quite noisy, add a second low pass digital filter stage */
#define ANALOG_FILTER_TICKS 14
#define ANALOG_FILTER_RESET 9
volatile uint8_t analog_tick = ANALOG_FILTER_TICKS;

static void burner(uint8_t enable)
{
  palWritePad(GPIOA, GPIOA_K1_FIRE, enable ? PAL_LOW : PAL_HIGH);
}

static uint8_t burnerState(void)
{
  return palReadPad(GPIOA, GPIOA_K1_FIRE) ? 1 : 0;
}

static void circulation(uint8_t enable)
{
  palWritePad(GPIOA, GPIOA_K2_20A, enable ? PAL_LOW : PAL_HIGH);
}

static uint8_t circulationState(void)
{
  return palReadPad(GPIOA, GPIOA_K2_20A) ? 1 : 0;
}

static uint8_t senseState(void)
{
  return palReadPad(GPIOB, GPIOB_SENSE) ? 1 : 0;
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
   * (following measurements in oct/2018 with averaging a lot of values)
   * basement 13-14: ** 33232 33334
   * 33289 (outside: 14.1 degrees)
   * 33276 (outside; 13.2 degrees)
   * 33261 (outside: 12.8 degrees)
   * 33213 (outside: 12.2 degrees)
   * 33056 (outside: ~10 degrees)
   * (results seem really bad :( )
   *
   * after changing analog filter, we get:
   * (outside):
   * 33422 for 14.3 degrees
   * 33371 for 13.2 degrees
   * 33066 for 9.9 degrees
   * 32888 for 6.1 degrees
   * 32875 for 5.3 degrees
   * (flow) 37240 for 75 degrees
   * (flow) 33320 for 12 degrees

   */
  float u = qfp_int2float(raw_adc);
  float t = qfp_fadd(coef->c0, qfp_fadd(qfp_fmul(coef->c1, u), qfp_fmul(qfp_fmul(u, u), coef->c2)));
  return qfp_float2int(t);
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

static void broadcast_heater_status(int16_t temp_out, int16_t temp_burner, int16_t temp_flow, uint8_t circulation, uint8_t burner, uint8_t burnerActuallyFiring, int16_t temp_flow_target)
{
  static uint8_t transfer_id = 0;
  uint8_t buffer[8];
  /* temperatures: -163.84 .. +163.83 -> 15 bit each */
  canardEncodeScalar(buffer, 0, 15, &temp_out);
  canardEncodeScalar(buffer, 15, 15, &temp_burner);
  canardEncodeScalar(buffer, 30, 15, &temp_flow);
  canardEncodeScalar(buffer, 45, 15, &temp_flow_target);
  canardEncodeScalar(buffer, 60, 1, &circulation);
  canardEncodeScalar(buffer, 61, 1, &burner);
  canardEncodeScalar(buffer, 62, 1, &burnerActuallyFiring);

  canardLockBroadcast(&canard,
      0x62cd3c11cab620ca,
      20008, &transfer_id,
      CANARD_TRANSFER_PRIORITY_LOW, buffer, sizeof(buffer));
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

  /* For the filter stage we rely on the analog tick being reset once per second (tick) */
  if(analog_tick < ANALOG_FILTER_TICKS)
  {
    if(analog_tick == ANALOG_FILTER_RESET)
    {
      analog_filter_reset((1 << 1) | (1 << 2) | (1 << 3));
    }
    if(analog_tick == ANALOG_FILTER_TICKS - 1)
    {
      if(adc_double_filtered[0] == 0) /* Filter has not been initialized */
      {
        adc_double_filtered[0] = adc_smp_filtered[1] * 65536;
        adc_double_filtered[1] = adc_smp_filtered[2] * 65536;
        adc_double_filtered[2] = adc_smp_filtered[3] * 65536;
      } else
      {
        /* Outside temp: low pass, fg ~ 1/3600 Hz */
        adc_double_filtered[0] = (((uint32_t)adc_smp_filtered[1] * 65536) + 256) / 512 + ((adc_double_filtered[0] + 256) / 512) * 511;
        /* heating system temp: low pass */
        adc_double_filtered[1] = (((uint32_t)adc_smp_filtered[2] * 65536) + 4) / 8 + ((adc_double_filtered[1] + 4) / 8) * 7;
        adc_double_filtered[2] = (((uint32_t)adc_smp_filtered[3] * 65536) + 4) / 8 + ((adc_double_filtered[2] + 4) / 8) * 7;
      }
      if(key[KEY_MODE] == KEY_MODE_DEBUG)
      {
        /* temporary debug raw temperature values */
        broadcast_heater_status(adc_double_filtered[0]/65536, adc_double_filtered[1]/65536, adc_double_filtered[2]/65536, circulationState(), burnerState(), 0, 0);
      } else {
        temperature[0] = pt500_adc_to_centicelsius(adc_double_filtered[0]/65536, &pt500[0]);
        temperature[1] = pt500_adc_to_centicelsius(adc_double_filtered[1]/65536, &pt500[1]);
        temperature[2] = pt500_adc_to_centicelsius(adc_double_filtered[2]/65536, &pt500[2]);
      }

      analog_pullup(0);
    }
    analog_tick++;
  }
}

/* Returns outside temperature in 1/100 degrees celsius */
static int16_t getOutsideTemperature(void)
{
  return temperature[0];
}

/* Returns target temperature in 1/100 degrees celsius */
static int16_t getTargetTemperature(uint8_t night)
{
  if(night)
  {
    return (7 + key[KEY_NIGHT]) * 100;
  } else {
    return (13 + key[KEY_DAY]) * 100;
  }

}

static float getCurveSlope(void)
{
  /* nearly fits the original, adds 1.3 to have more resolution in interesting area */
  static const float slope[] = {0.2f, 0.4f, 0.6f, 0.7f, 0.8f, 0.9f, 1.0f, 1.1f, 1.2f, 1.3f, 1.4f, 1.6f, 1.9f, 2.2f, 2.4f, 2.6f};
  return slope[key[KEY_SLOPE] & 0x0F];
}

static float getCurveOffset(void)
{
  return qfp_int2float((int16_t)key[KEY_OFFSET] * 3 - 12);
}

static int16_t getFlowTemperature(void)
{
  return temperature[2];
}

static int16_t getBurnerTemperature(void)
{
  return temperature[1];
}

/* calculates the target burner temperature
 * returns it in 1/100 degrees celsius
 */
int16_t getTargetFlowTemperature(int16_t targetTemperature)
{
  float outside = qfp_fdiv(qfp_int2float(getOutsideTemperature()), 100);
  float target = qfp_fdiv(qfp_int2float(targetTemperature), 100);

  /* KT = neigung * 1.8317984 * (raumsoll-aussentemp)^0.8281902 + niveau + raumsoll */
  /* 3x mul (165) + 1x sub(151) + 2x add (150) + 1x ln (829) + 1x exp (557)
   * this function might need about 3000 clock cycles total, equalling ~63µs */
  float exp = qfp_fmul(0.8281902f, qfp_fln(qfp_fsub(target, outside)));
  float result = qfp_fadd(qfp_fadd(
      qfp_fmul(getCurveSlope(),
      qfp_fmul(1.8317984f,
          qfp_fexp(exp))),
      getCurveOffset()), target);


  return qfp_float2int(qfp_fmul(result, 100));
}

/* calculates an exponentially weighted difference of the actual and the target flow temperature
to properly account for higher/lower flow temperatures during firing period. */
int32_t getWeightedFlowDeviation(int16_t targetTemperature, int16_t targetFlowTemperature, int16_t actualFlowTemperature)
{
  if(targetFlowTemperature <= targetTemperature) {
    return 0;
  }
  /* our radiators have an exponential factor of approx. 1.3 */
  const float radiatorExponent = 1.3f;
  const float target = qfp_fdiv(qfp_int2float(targetTemperature), 100);
  const float targetFlow = qfp_fdiv(qfp_int2float(targetFlowTemperature), 100);
  const float actualFlow = qfp_fdiv(qfp_int2float(actualFlowTemperature), 100);
  const float targetProportionalPower = qfp_fexp(qfp_fmul(qfp_fln(qfp_fsub(targetFlow, target)), radiatorExponent));
  if(actualFlowTemperature <= targetTemperature) {
    return qfp_float2int(qfp_fmul(targetProportionalPower, -100));
  }
  const float actualProportionalPower = qfp_fexp(qfp_fmul(qfp_fln(qfp_fsub(actualFlow, target)), radiatorExponent));
  return qfp_float2int(qfp_fmul(qfp_fsub(actualProportionalPower, targetProportionalPower), 100));
}


static void read_config(void)
{
  flowHysteresisTurnOff = config_get_int(CONFIG_HEATER_FLOW_HYSTERESIS_TURN_OFF);
  flowHysteresisTurnOn = config_get_int(CONFIG_HEATER_FLOW_HYSTERESIS_TURN_ON);
  minFlowTempToEnable = config_get_uint(CONFIG_HEATER_FLOW_MIN_TARGET_TEMP);
  minTempToEnableCirculation = config_get_uint(CONFIG_HEATER_FLOW_MIN_CIRCULATION_TEMP);
  burnerStopTempRiseTime = config_get_uint(CONFIG_HEATER_BURNER_TEMP_RISE_MIN_TIME);
  burnerMinOffTime = config_get_uint(CONFIG_HEATER_BURNER_MIN_OFF_TIME);
  circulationMinOnTimeForThreshold = config_get_uint(CONFIG_HEATER_CIRCULATION_MIN_ON_FOR_THRESHOLD);
  time1Start = config_get_uint(CONFIG_HEATER_TIME_1_START);
  time1Stop = config_get_uint(CONFIG_HEATER_TIME_1_STOP);
  time1Temp = config_get_int(CONFIG_HEATER_TIME_1_TEMP);
  time2Temp = config_get_int(CONFIG_HEATER_TIME_2_TEMP);
  deviationHysteresis = config_get_int(CONFIG_HEATER_DEVIATION_HYSTERESIS);
}

static uint8_t inTimeRange(uint16_t start, uint16_t stop)
{
  uint8_t current_h = time_hour;
  uint8_t current_m = time_minute;
  uint8_t start_h = (start >> 8) & 0x0FF;
  uint8_t start_m = start & 0x0FF;
  uint8_t stop_h = (stop >> 8) & 0x0FF;
  uint8_t stop_m = stop & 0x0FF;

  if(current_h > start_h && current_h < stop_h)
  {
    return 1;
  }
  if(current_h == start_h && current_m >= start_m)
  {
    return 1;
  }
  if(current_h == stop_h && current_m < stop_m)
  {
    return 1;
  }
  return 0;
}

void app_tick(void)
{
  static uint8_t led_count = 0;
  analog_pullup(1);
  analog_tick = 0;
  led_a(led_count & 1);
  led_b(led_count & 2);
  led_count++;
  int16_t targetFlowTemp;
  int16_t flowTemp = getFlowTemperature();
  int16_t burnerTemp = getBurnerTemperature();
  int16_t targetTemp;
  static uint8_t currentBurnerState;
  static uint8_t currentCirculationState;
  static systime_t lastBurnerRequestStart;
  static systime_t lastBurnerStop = 0x8FF00;    /* Future initialization so we don't start in burner stop phase */
  static systime_t lastCirculationStart;
  static int16_t extendThisCycleFlowTemp;
  /* total deviation (integral) from target flow temperature in kelvin multiplied by seconds (given by tick interval) */
  static int32_t flowTempDeviation;
  uint8_t nextBurnerState = 0;
  uint8_t nextCirculationState = 0;
  int16_t burnerLidFor = (currentBurnerState ? TIME_I2S(chVTTimeElapsedSinceX(lastBurnerRequestStart)) : 0) - BURNER_REQUEST_DELAY;
  int32_t circulationRunningFor = (currentCirculationState ? TIME_I2S(chVTTimeElapsedSinceX(lastCirculationStart)) : 0);
  if(inTimeRange(time1Start, time1Stop))
  {
    targetTemp = getTargetTemperature(0) + time1Temp;
  } else {
    targetTemp = getTargetTemperature(1) + time2Temp;
  }
  targetFlowTemp = getTargetFlowTemperature(targetTemp);

  if(key[KEY_MODE] == KEY_MODE_DEBUG)
  {
    nextBurnerState = key[KEY_SLOPE] & 1;
    nextCirculationState = key[KEY_SLOPE] & 2;
  } else if(key[KEY_MODE] == KEY_MODE_DAY_NIGHT)
  {
    node_debug_int(LOG_LEVEL_INFO, "FLOWTARGET", targetFlowTemp);
    node_debug_int(LOG_LEVEL_INFO, "FLOWTEMP", flowTemp);
    node_debug_int(LOG_LEVEL_INFO, "FLOWDEVIATION", flowTempDeviation);
    node_debug_int(LOG_LEVEL_INFO, "OFFSET", qfp_float2int(getCurveOffset()));
    node_debug_int(LOG_LEVEL_INFO, "SLOPE", qfp_float2int(qfp_fmul(10.0f, getCurveSlope())));
    flowTempDeviation += getWeightedFlowDeviation(targetTemp, targetFlowTemp, flowTemp);
    /* limit deviation integral. Factor 8 chosen arbitrarily */
    if(flowTempDeviation > 8*deviationHysteresis) {
      flowTempDeviation = 8*deviationHysteresis;
    } else if(flowTempDeviation < -8*deviationHysteresis) {
      flowTempDeviation = -8*deviationHysteresis;
    }
    
    if((currentCirculationState && flowTemp > (minTempToEnableCirculation - 100)) ||
      (flowTemp > minTempToEnableCirculation) ||
      (burnerTemp > minTempToEnableCirculation)
      )
    {
      nextCirculationState = 1;
    }
    if(targetFlowTemp > minFlowTempToEnable)
    {
      nextBurnerState = currentBurnerState;
      if(!currentBurnerState)
      {
        /* flow temp controlled */
        if(!deviationHysteresis && (flowTemp < (targetFlowTemp - flowHysteresisTurnOn)))
        {
          nextBurnerState = 1;
        }
        /* flow temp deviation controlled */
        if(deviationHysteresis && (flowTempDeviation < (0 - deviationHysteresis))) {
          nextBurnerState = 1;
        }
      }
      if(currentBurnerState)
      {
        if((flowTemp > (targetFlowTemp + flowHysteresisTurnOff)
        && (flowTemp > (targetFlowTemp + extendThisCycleFlowTemp))) ||  // cycle extension can override (increase) turn off hysteresis
           (burnerTemp > (targetFlowTemp + flowHysteresisTurnOff)
        && (burnerTemp > targetFlowTemp + extendThisCycleFlowTemp)))
        {
          nextBurnerState = 0;
        }
        if(deviationHysteresis && (flowTempDeviation > deviationHysteresis)) {
          /* additionally, turn off when deviation reached positive turn off point */
          nextBurnerState = 0;
        }
        if(burnerTemp > flowTemp && burnerLidFor > burnerStopTempRiseTime && circulationRunningFor > circulationMinOnTimeForThreshold)
        {
          nextBurnerState = 0;
        }
      }
    } else {
      /* reset deviation because we don't try to heat */
      flowTempDeviation = 0;
    }
  } else if(key[KEY_MODE] == KEY_MODE_DAY_OFF)
  { /* this mode is temporarily used to "shut down" in the night */
    flowTempDeviation = 0;
    if(flowTemp > minTempToEnableCirculation || burnerTemp > minTempToEnableCirculation)
    { /* Let circulation run until heating system cooled down */
      nextCirculationState = 1;
    }
  }
  if(!nextBurnerState && currentBurnerState) {
    lastBurnerStop = chVTGetSystemTimeX();
  }
  if(nextBurnerState && !currentBurnerState) {
    if(TIME_I2S(chVTTimeElapsedSinceX((lastBurnerStop))) < burnerMinOffTime)
    {
      /* disable burner request in off time */
      nextBurnerState = 0;
    } else {
      /* Detection of burner request */
      lastBurnerRequestStart = chVTGetSystemTimeX();
      extendThisCycleFlowTemp = targetFlowTemp - flowTemp;
      if(extendThisCycleFlowTemp > (2 * flowHysteresisTurnOff)) {
        extendThisCycleFlowTemp = 2 * flowHysteresisTurnOff;
      }      
    }
  }
  if(nextCirculationState && !currentCirculationState) {
    lastCirculationStart = chVTGetSystemTimeX();
  }
  burner(nextBurnerState);
  /* actual burning starts 50 seconds after start signal (Abgasklappe, Feuerungsautomat) */
  /* gas usage for 32 kW Atola at Kanthaus: 0.05m³ in 50 seconds -> 0.001m³/s or 3.6m³/h
   * gas usage measured in a 50 second interval by looking at counter. Measuring start a few seconds after burner start. */
  circulation(nextCirculationState);
  broadcast_heater_status(temperature[0], temperature[1], temperature[2], nextCirculationState, nextBurnerState, burnerLidFor > 0, targetFlowTemp);
  currentBurnerState = nextBurnerState;
  currentCirculationState = nextCirculationState;
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

  read_config();
}

void app_config_update(void)
{
  read_config();
}


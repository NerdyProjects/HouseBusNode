/*
 * This is the bathroom node with following planned functionality:
 * * Humidity Sensor
 * * Ventilation controller (230V dedicated fan)
 * * Light controller (Default not so bride, optionally bright)
 * * Occupancy Indicator (Motion sensor, door sensor, "privately occupied" mode)
 *
 * Pinout:
 * PA0 - LDR against GND, 4k7 Pullup to 3.3V
 * PA2 - Occupancy red
 * PA3 - Occupancy yellow
 * PA4 - Occupancy green
 * PA5 - Occupancy switch
 * PA6 - Mosfet to GND (soft light)
 * PA7 - Mosfet to GND, 2k4 Pullup to Vin (piezo)
 * PB1 - Light switch
 * PB2 - Motion sensor
 * PB3 - 230V Switch J47 (Bright light)
 * PB4 - 230V Switch J46 (Fan)
 * PB10 - door sensor
 *
 * Hardware notes:
 * Both 230V switches have only MOC3063, 2x 390 Ohms, BTA212-800B and 275V/0.6W Varistor in place
 * 
 * Indicator Panel (outside; 3x LED + Piezzo vs GND):
 * * Red LED 200 Ohm red
 * * Yellow LED 68 Ohm yellow
 * * Green LED 68 Ohm blue
 * * Piezo white
 *
 * Control Panel (inside; 2x switch + 2x LED vs GND)
 * * Red LED 200 Ohm red
 * * Yellow LED 68 Ohm yellow
 * * Switch (Occupancy, left) blue
 * * Switch (light, right) white
 */

#include "node.h"
#include "hal.h"
#include "qfplib.h"
#include "drivers/analog.h"
#include "modules/bme280.h"

#define LED_RED 0
#define LED_YELLOW 1
#define LED_GREEN 2

#define KEY_OCCUPANCY 0

#define MOTION_TRIGGER_ACTIVE_S 45
#define MOTION_TRIGGER_ACTIVE_DOOR_CLOSED_S 240
#define MOTION_TRIGGER_AFTER_DOOR_CLOSE_MIN_S 5
#define PRIVATE_MODE_BUTTON_VALID_FOR_S 30

static systime_t motion_sensor_last_active;
static uint8_t motion_sensor_state;
static uint8_t door_closed;
static systime_t door_sensor_last_open;
static uint16_t ambientBrightness;
static uint8_t blinkTicks;
static uint8_t blinkLedNr;
static uint8_t blinkSpeed;
static uint8_t blinkState;
static uint8_t occupancySwitchPressed;

/* sets LED 0-2 to given state */
static void setLed(uint8_t led, uint8_t on)
{
  if(led < 3) {
    uint8_t pin = led + 2; /* Map to PA2-PA4 */
    if(on >= 1) {
      on = 1;
    }
    palWritePad(GPIOA, pin, on);
  }
}

void app_init(void)
{
  palSetPadMode(GPIOA, 5, PAL_MODE_INPUT_PULLUP); /* PA5 Switch */
  palSetGroupMode(GPIOA,
      PAL_PORT_BIT(2) |  /* PA2-PA4 LEDs */
      PAL_PORT_BIT(3) |
      PAL_PORT_BIT(4) |
      PAL_PORT_BIT(6) |   /* PA6 Light */
      PAL_PORT_BIT(7)     /* PA7 Piezo (will be overriden by tone generator PWM) */
      , 0, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOA, 0, PAL_MODE_INPUT_ANALOG); /* PA0 LDR */
  palSetGroupMode(GPIOB,
      PAL_PORT_BIT(1) |   /* PB1 Light switch */
      PAL_PORT_BIT(10)    /* PB10 Door sensor */
      , 0, PAL_MODE_INPUT_PULLUP);
  palSetGroupMode(GPIOB,
      PAL_PORT_BIT(3) |   /* PB3-4 230V switch light/fan */
      PAL_PORT_BIT(4)
      , 0, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOB, 2, PAL_MODE_INPUT_PULLDOWN); /* PB2 Motion sensor */

  setLed(LED_RED, 0);
  setLed(LED_GREEN, 0);
  setLed(LED_YELLOW, 0);
  palClearPad(GPIOA, 7);
  bme280_app_init();
}

static void readMotionSensor(void)
{
  if(palReadPad(GPIOB, 2))
  {
    motion_sensor_last_active = chVTGetSystemTimeX();
  }
  motion_sensor_state = chVTTimeElapsedSinceX(motion_sensor_last_active) < TIME_S2I(MOTION_TRIGGER_ACTIVE_S);
}

static void readButtons(void)
{
  uint8_t lightSwitch = palReadPad(GPIOB, 1);
  uint8_t occupancySwitch = palReadPad(GPIOA, 5);
  uint8_t doorSensor = palReadPad(GPIOB, 10);
  static uint8_t doorSensorLast;
  static uint8_t doorSensorCount;
  if(doorSensor == doorSensorLast) {
    if(doorSensorCount > 100) {
      door_closed = !doorSensor;
      if(!door_closed) {
        door_sensor_last_open = chVTGetSystemTimeX();
      }
    } else {
      doorSensorCount++;
    }
  } else {
    doorSensorCount = 0;
  }
  doorSensorLast = doorSensor;

  {
    static uint8_t occupancySwitchLast;
    static uint8_t occupancySwitchCount;
    if(occupancySwitch == occupancySwitchLast) {
      if(occupancySwitchCount == 10 && !occupancySwitch) {
        occupancySwitchPressed = 1;
      }
      if(occupancySwitchCount <= 10) {
        occupancySwitchCount++;
      }
    } else {
      occupancySwitchCount = 0;
    }
  }
}

static void readLdr(void)
{

}

static uint8_t readKey(uint8_t key) {
  if(key == KEY_OCCUPANCY && occupancySwitchPressed) {
    occupancySwitchPressed = 0;
    return 1;
  }
  return 0;
}

/* controls blink mode: speed = 0 for turning blinking off */
static void blinkLed(uint8_t number, uint8_t speed) {
  if(blinkLedNr != number && blinkSpeed != speed) {
    blinkLedNr = number;
    blinkSpeed = speed;
    blinkState = 1;
    setLed(number, blinkSpeed); /* only turn on if a speed is set */
  }
}


typedef enum {
  OCCUPANCY_MODE_EMPTY,
  OCCUPANCY_MODE_OPEN_IN_USE,
  OCCUPANCY_MODE_IN_USE,
  OCCUPANCY_MODE_PRIVATE
} occupancyMode;

static void occupancyIndicatorTick(void)
{
  static occupancyMode state = OCCUPANCY_MODE_EMPTY;
  static systime_t occupancyPressedAt;
  occupancyMode nextState = state;
  uint32_t motionSensorActiveAgo = chVTTimeElapsedSinceX(motion_sensor_last_active);
  uint32_t doorOpenAgo = chVTTimeElapsedSinceX(door_sensor_last_open);
  static uint8_t motion_sensor_active_since_door_closed;

  if(door_closed && !motion_sensor_active_since_door_closed){
    if(doorOpenAgo > TIME_S2I(MOTION_TRIGGER_AFTER_DOOR_CLOSE_MIN_S) && motionSensorActiveAgo < (doorOpenAgo - TIME_S2I(MOTION_TRIGGER_AFTER_DOOR_CLOSE_MIN_S))) {
      motion_sensor_active_since_door_closed = 1;
    }
  } else if(!door_closed) {
    motion_sensor_active_since_door_closed = 0;
  }

  if(readKey(KEY_OCCUPANCY))
  {
    occupancyPressedAt = chVTGetSystemTimeX();
    if(occupancyPressedAt == 0) {
      occupancyPressedAt = 1;
    }
  }

  if(chVTTimeElapsedSinceX(occupancyPressedAt) > TIME_S2I(PRIVATE_MODE_BUTTON_VALID_FOR_S)) {
    occupancyPressedAt = 0;
  }

  switch(state)
  {
  case OCCUPANCY_MODE_EMPTY:
    node_debug(LOG_LEVEL_INFO, "BATH", "empty");
    /* Room is not empty, when a motion is detected inside :-) */
    if(motionSensorActiveAgo < TIME_S2I(2)) {
      nextState = OCCUPANCY_MODE_OPEN_IN_USE;
    }
    /* other state changes are handled from OPEN_IN_USE state */
    break;
  case OCCUPANCY_MODE_OPEN_IN_USE:
    node_debug(LOG_LEVEL_INFO, "BATH", "open in use");
    if(door_closed && motion_sensor_active_since_door_closed) {
      nextState = OCCUPANCY_MODE_IN_USE;
    }
    if(motionSensorActiveAgo > TIME_S2I(MOTION_TRIGGER_ACTIVE_S)) {
      nextState = OCCUPANCY_MODE_EMPTY;
    }
    break;
  case OCCUPANCY_MODE_IN_USE:
    node_debug(LOG_LEVEL_INFO, "BATH", "in use");
    if(occupancyPressedAt) {
      nextState = OCCUPANCY_MODE_PRIVATE;
      occupancyPressedAt = 0;
    } else {
      if(!door_closed) {
        nextState = OCCUPANCY_MODE_OPEN_IN_USE;
      }
      if((motionSensorActiveAgo > TIME_S2I(MOTION_TRIGGER_ACTIVE_DOOR_CLOSED_S) && motion_sensor_active_since_door_closed)
          || (motionSensorActiveAgo > TIME_S2I(MOTION_TRIGGER_ACTIVE_S) && !motion_sensor_active_since_door_closed)) {
        nextState = OCCUPANCY_MODE_EMPTY;
      }
    }
    break;
  case OCCUPANCY_MODE_PRIVATE:
    if(!door_closed) {
      nextState = OCCUPANCY_MODE_OPEN_IN_USE;
    }
    if(motionSensorActiveAgo > TIME_S2I(MOTION_TRIGGER_ACTIVE_DOOR_CLOSED_S)) {
      nextState = OCCUPANCY_MODE_EMPTY;
    }
    if(occupancyPressedAt) {
      nextState = OCCUPANCY_MODE_IN_USE;
    }
    break;
  default:
    break;
  }
  state = nextState;
  switch (state) {
    case OCCUPANCY_MODE_EMPTY:
      setLed(LED_GREEN, 1);
      setLed(LED_YELLOW, 0);
      setLed(LED_RED, 0);
      blinkLed(LED_YELLOW, 0);
      break;
    case OCCUPANCY_MODE_IN_USE:
      setLed(LED_GREEN, 0);
      setLed(LED_YELLOW, 1);
      setLed(LED_RED, 0);
      blinkLed(LED_YELLOW, 0);
      break;
    case OCCUPANCY_MODE_OPEN_IN_USE:
      setLed(LED_GREEN, 0);
      blinkLed(LED_YELLOW, 70);
      setLed(LED_RED, 0);
      break;
    case OCCUPANCY_MODE_PRIVATE:
      setLed(LED_GREEN, 0);
      blinkLed(LED_YELLOW, 0);
      setLed(LED_RED, 1);
    default:
      break;
  }
}

static int16_t calculateDewPoint(int32_t centiTemperature, uint32_t milliHumidity) {
  /**
   * if( $T >= 0 ){$a=7.5; $b=237.3;}
else{$a=7.6; $b=240.7;}
$sdd = 6.1078 * pow(10.0, (($a*$T)/($b+$T))); // Magnusformel
$dd=($F/100.0) * $sdd;
$v=log10(($dd/6.1078));
$td=($b*$v)/($a-$v);
   */
  float temp = qfp_fdiv(qfp_int2float(centiTemperature), 100);
  float rh = qfp_fdiv(qfp_int2float(milliHumidity), 1000);
  float a = 7.5f;
  float b = 237.3f;
  float acc = qfp_fdiv(qfp_fmul(a, temp), qfp_fadd(b, temp));
  float sdd = qfp_fexp(qfp_fmul(acc, 2.30258509299f)); /* sdd excludes factor 6.1078 */
  float dd = qfp_fmul(rh, sdd); /* dd excludes factor 6.1078 from sdd */
  float v = qfp_fdiv(qfp_fln(dd), 2.30258509299f);
  float res = qfp_fdiv(qfp_fmul(b, v), qfp_fsub(a, v));


  return qfp_float2int(qfp_fmul(res, 100));


  /* fH = (math.log10(humidity) - 2) / 0.4343 + (17.62 * temperature) / (243.12 + temperature)
    dewpoint = 243.12 * fH / (17.62 - fH) */

    /*
     * Bezeichnungen:
r = relative Luftfeuchte
T = Temperatur in °C
TK = Temperatur in Kelvin (TK = T + 273.15)
TD = Taupunkttemperatur in °C
DD = Dampfdruck in hPa
SDD = Sättigungsdampfdruck in hPa

Parameter:
a = 7.5, b = 237.3 für T >= 0
a = 7.6, b = 240.7 für T < 0 über Wasser (Taupunkt)
a = 9.5, b = 265.5 für T < 0 über Eis (Frostpunkt)
     *
     * SDD(T) = 6.1078 * 10^((a*T)/(b+T))
DD(r,T) = r/100 * SDD(T)
r(T,TD) = 100 * SDD(TD) / SDD(T)
TD(r,T) = b*v/(a-v) mit v(r,T) = log10(DD(r,T)/6.1078)
    *
    * Taupunkt = 100 * 6.1078 * 10^(7.5*T)/(237.3+T)
    *
    * 237.3*(log10(r/100 * 6.1078 * 10^((7.5*T)/(237.3+T)))/6.1078
     */
}

static void fanControl(void)
{

}

/*
 * LDR: 59000 soft LED light, 64900 dark, 21000 on a light winter day lunchtime
 */

void app_tick(void)
{
  uint8_t dbgbuf[20];
  chsnprintf(dbgbuf, 20, "%d", adc_smp_filtered[0]);
  node_debug(LOG_LEVEL_INFO, "LDR", dbgbuf);
  bme280_app_read();
  readMotionSensor();
  readLdr();
  occupancyIndicatorTick();
}

void app_fast_tick(void)
{
  readButtons();
  if(blinkSpeed) {
    if(++blinkTicks >= blinkSpeed) {
      blinkState = !blinkState;
      setLed(blinkLedNr, blinkState);
      blinkTicks = 0;
    }
  }
}

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
#include "config.h"
#include "drivers/analog.h"
#include "modules/bme280.h"
#include "modules/temperature_receiver.h"
#include "modules/dewpoint.h"
#include "chprintf.h"
#include "dsdl/homeautomation/BathroomStatus.h"

#define LED_RED 0
#define LED_YELLOW 1
#define LED_GREEN 2

#define KEY_OCCUPANCY 0

#define MOTION_TRIGGER_ACTIVE_S 45
#define MOTION_TRIGGER_ACTIVE_DOOR_CLOSED_S 900
#define MOTION_TRIGGER_BLOCK_AFTER_DOOR_CLOSE_S 4
#define MOTION_TRIGGER_AFTER_DOOR_CLOSE_NOT_EMPTY_S 10
#define PRIVATE_MODE_BUTTON_VALID_FOR_S 30
#define PRIVATE_MODE_EMPTY_VALID_FOR_S 7

#define LDR_TICK_INTERVAL_S 10

static systime_t motion_sensor_last_active;
static uint8_t motion_sensor_state;
static uint8_t door_closed;
static systime_t door_sensor_last_open;
static uint8_t blinkTicks;
static volatile uint8_t blinkLedNr;
static volatile uint8_t blinkSpeed;
static uint8_t blinkState;
static uint8_t occupancySwitchPressed;
static uint8_t brightnessSwitchPressed;

static uint16_t wallTemperatureFactor;
static uint32_t fanOnAboveRelativeHumidity;
static sysinterval_t fanRunOutTime;

typedef homeautomation_BathroomStatus BathroomStatus;

static BathroomStatus bathroomStatus;

/* sets the light: 0 - off, 1 - low, 2 - high */
static void setLight(uint8_t brightness)
{
  switch(brightness)
  {
  case 0:
    palWritePad(GPIOB, 3, 0);
    palWritePad(GPIOA, 6, 0);
    break;
  case 1:
    palWritePad(GPIOB, 3, 0);
    palWritePad(GPIOA, 6, 1);
    break;
  case 2:
    palWritePad(GPIOB, 3, 1);
    palWritePad(GPIOA, 6, 1);
    break;
  }
}

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
    uint8_t occupancySwitch = palReadPad(GPIOA, 5);
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

  {
    uint8_t brightnessSwitch = palReadPad(GPIOB, 1);
    static uint8_t brightnessSwitchLast;
    static uint8_t brightnessSwitchCount;
    if(brightnessSwitch == brightnessSwitchLast) {
      if(brightnessSwitchCount == 10 && !brightnessSwitch) {
        brightnessSwitchPressed = 1;
      }
      if(brightnessSwitchCount <= 10) {
        brightnessSwitchCount++;
      }
    } else {
      brightnessSwitchCount = 0;
    }
  }
}

/*
 * this method calculates a moving average through a window of recent brightnesses
 * and stores it into the status field.
 * LDR: 59000 soft LED light, 64900 dark, 21000 on a light winter day lunchtime
 */
static void readLdr(BathroomStatus *status)
{
#define LDR_WINDOW_LENGTH 128
  static uint8_t window[LDR_WINDOW_LENGTH];
  static uint8_t windowPointer;
  static uint16_t sum;
  static systime_t lastReadingAt;
  if(chVTTimeElapsedSinceX(lastReadingAt) >= TIME_S2I(LDR_TICK_INTERVAL_S)) {
    /* scale from 65535..0 to 0..255 dark..bright */
    uint8_t v = (~adc_smp_filtered[0]) >> 8;
    if(status->person_inside) {
      /* possibility of non-natural lighting; just use previous brightness again */
      v = window[windowPointer];
    }
    windowPointer = (windowPointer + 1) % LDR_WINDOW_LENGTH;
    sum += v - window[windowPointer];
    window[windowPointer] = v;

    status->brightness = (sum / LDR_WINDOW_LENGTH) >> (5); /* status structure is 3 bits wide */
    lastReadingAt = chVTGetSystemTimeX();
  }
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
  if(blinkLedNr != number || blinkSpeed != speed) {
    blinkLedNr = number;
    blinkSpeed = speed;
    if(blinkSpeed)
    {
      blinkState = 1;
      setLed(number, blinkSpeed); /* only turn on if a speed is set */
    }
  }
}


typedef enum {
  OCCUPANCY_MODE_EMPTY,
  OCCUPANCY_MODE_OPEN_IN_USE,
  OCCUPANCY_MODE_IN_USE,
  OCCUPANCY_MODE_PRIVATE
} occupancyMode;

static void occupancyIndicatorTick(BathroomStatus *status)
{
  static occupancyMode state = OCCUPANCY_MODE_EMPTY;
  static systime_t occupancyPressedAt;
  occupancyMode nextState = state;
  uint32_t motionSensorActiveAgo = chVTTimeElapsedSinceX(motion_sensor_last_active);
  uint32_t doorOpenAgo = chVTTimeElapsedSinceX(door_sensor_last_open);
  static uint8_t motion_sensor_active_since_door_closed;
  static uint8_t door_open_counter; /* for OCCUPANCY_MODE_PRIVATE */
  /* detection of behaviour: Motion sensor should have sensed activity IF there was any */
  uint8_t motion_sensor_should_have_been_active_since_door_closed =
      door_closed && doorOpenAgo > TIME_S2I(MOTION_TRIGGER_AFTER_DOOR_CLOSE_NOT_EMPTY_S);

  if(door_closed && !motion_sensor_active_since_door_closed){
    if(doorOpenAgo >= TIME_S2I(MOTION_TRIGGER_BLOCK_AFTER_DOOR_CLOSE_S) && motionSensorActiveAgo < (doorOpenAgo - TIME_S2I(MOTION_TRIGGER_BLOCK_AFTER_DOOR_CLOSE_S))) {
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
    /* Room is not empty, when a motion is detected inside :-) */
    if(motionSensorActiveAgo < TIME_S2I(2)) {
      nextState = OCCUPANCY_MODE_OPEN_IN_USE;
    }
    /* other state changes are handled from OPEN_IN_USE state */
    break;
  case OCCUPANCY_MODE_OPEN_IN_USE:
    if(door_closed && motion_sensor_active_since_door_closed) {
      nextState = OCCUPANCY_MODE_IN_USE;
    }
    if(motionSensorActiveAgo > TIME_S2I(MOTION_TRIGGER_ACTIVE_S)) {
      nextState = OCCUPANCY_MODE_EMPTY;
    }
    if(occupancyPressedAt) {
      occupancyPressedAt = 0;
      nextState = OCCUPANCY_MODE_PRIVATE;
    }
    if(door_closed && !motion_sensor_active_since_door_closed &&
        motion_sensor_should_have_been_active_since_door_closed) {
    /* fast exit of this state to empty when door is closed and no person detected */
      nextState = OCCUPANCY_MODE_EMPTY;
    }

    break;
  case OCCUPANCY_MODE_IN_USE:
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
      /* allow a short time of door open in private mode */
      if(++door_open_counter >= PRIVATE_MODE_EMPTY_VALID_FOR_S) {
        nextState = OCCUPANCY_MODE_OPEN_IN_USE;
      }
    } else {
      door_open_counter = 0;
    }
    if(motionSensorActiveAgo > TIME_S2I(MOTION_TRIGGER_ACTIVE_DOOR_CLOSED_S) ||
       (motion_sensor_should_have_been_active_since_door_closed && !motion_sensor_active_since_door_closed)) {
      nextState = OCCUPANCY_MODE_EMPTY;
    }
    if(occupancyPressedAt) {
      nextState = OCCUPANCY_MODE_IN_USE;
      occupancyPressedAt = 0;
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
      setLed(LED_YELLOW, 0);
      setLed(LED_RED, 1);
    default:
      break;
  }
  status->private_mode = (state == OCCUPANCY_MODE_PRIVATE);
  status->person_inside = (state != OCCUPANCY_MODE_EMPTY);
  status->door_open = !door_closed;
}

static void fanControlTick(BathroomStatus *status)
{
  static systime_t fanNotTriggeredAnymoreAt;
  /* 1: running, not triggered 2: running, triggered */
  static uint8_t fanRunning;
  uint8_t fanTrigger = 0;
  int16_t wallTemperatureDifference;
  uint32_t insideHumidity;
  if(bme_presence & 1)
  {
    insideHumidity = BMEMilliHumidity[0];
  } else {
    insideHumidity = BMEMilliHumidity[1];
  }
  if(insideHumidity > fanOnAboveRelativeHumidity)
  {
    fanTrigger = 1;
  }
  if(approximateWallTemperatureDifferenceToDewPoint(&wallTemperatureDifference, wallTemperatureFactor) == 0)
  {
    if(wallTemperatureDifference < 0) {
      fanTrigger = 1;
    }
  }

  if(fanTrigger)
  {
    fanRunning = 2;
    palSetPad(GPIOB, 4);
  }
  if(!fanTrigger && fanRunning == 2)
  {
    fanRunning = 1;
    fanNotTriggeredAnymoreAt = chVTGetSystemTimeX();
  }
  if(!fanTrigger && fanRunning && chVTTimeElapsedSinceX(fanNotTriggeredAnymoreAt) > fanRunOutTime)
  {
    fanRunning = 0;
    palClearPad(GPIOB, 4);
  }
  status->fan_running = fanRunning ? 1 : 0;
}

static void read_config(void)
{
  uint32_t t = config_get_uint(CONFIG_TEMPERATURE_RECEIVER_TARGET_NODE_ID);
  setReceiverTarget(t);
  wallTemperatureFactor = config_get_uint(CONFIG_WALL_TEMPERATURE_FACTOR_BY_1024);
  fanOnAboveRelativeHumidity = config_get_uint(CONFIG_FAN_ON_ABOVE_MILLI_RELATIVE_HUMIDITY);
  fanRunOutTime = TIME_S2I(config_get_uint(CONFIG_FAN_RUN_OUT_TIME_S));
}

static void bathroom_status_broadcast(BathroomStatus *status)
{
  static uint8_t transfer_id = 0;
  uint8_t buffer[HOMEAUTOMATION_BATHROOMSTATUS_MAX_SIZE];
  homeautomation_BathroomStatus_encode(status, buffer);
  /* the status bitfield is already in line format */
  canardLockBroadcast(&canard,
        HOMEAUTOMATION_BATHROOMSTATUS_SIGNATURE,
        HOMEAUTOMATION_BATHROOMSTATUS_ID, &transfer_id,
        CANARD_TRANSFER_PRIORITY_LOW, buffer, HOMEAUTOMATION_BATHROOMSTATUS_MAX_SIZE);
}

static void lightTick(BathroomStatus *status)
{
  static uint8_t bright = 0;
  uint8_t targetBrightness = 0;
  if (brightnessSwitchPressed)
  {
    if(status->person_inside) {
      /* only accept button press when somebody is in the room */
      bright = !bright;
    }
    brightnessSwitchPressed = 0;
  }
  if(status->person_inside) {
    if(status->brightness == 0) {
      targetBrightness = 1;
    }
    if(bright && status->brightness <= 3)
    {
      /* only offer bright light when natural lighting is not bright enough */
      targetBrightness = 2;
    }
  } else {
    /* light is only available when a person is in the room.
     * Also, reset to non-bright when room is empty
     */
    targetBrightness = 0;
    bright = 0;
  }
  setLight(targetBrightness);
}

void app_tick(void)
{
  bme280_app_read();
  readMotionSensor();
  readLdr(&bathroomStatus);
  occupancyIndicatorTick(&bathroomStatus);
  fanControlTick(&bathroomStatus);
  lightTick(&bathroomStatus);
  bathroom_status_broadcast(&bathroomStatus);
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

void app_config_update(void)
{
  read_config();
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
  read_config();
}

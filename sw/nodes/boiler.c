/* boiler node:
  Idea is to use excess solar energy, identified by OBIS 16.7.0, to heat up water.
  For that, there is a TRIAC driven via PB1 (configure the dimmer module accordingly).
  Also, there is a 47k NTC thermistor (K=4090) towards 3.3V connected at PA0. There is a 47k pulldown to GND as well.
  
  Thermistor resistance table:
  degrees resistance ~ADC val
  0 155.8k 15185
  10 94.53k 21759
  20 58.95k 29065
  25 47k 32760
  40 24.7k 42948
  60 11.3k 52820
  80 5.573k 58574
  90 4.015k 60363
  100 2.936k 61668
  */


#include "ch.h"
#include "node.h"
#include "dsdl/homeautomation/Obis.h"
#include "dsdl/homeautomation/BoilerStatus.h"
#include "modules/pid.h"
#include "modules/bme280.h"
#include "config.h"
#include "chprintf.h"
#include "dimmer.h"
#include "drivers/analog.h"
#include "qfplib.h"

/* if no controller output is generated in that time in seconds (e.g. no obis message received), turn off the boiler */
#define CONTROLLER_TIMEOUT 15
/* if no packet from other node received in that number of seconds consider it as offline */
#define OTHER_NODE_TIMEOUT 15

#define MAX_OTHER_BOILER_NODES 3

typedef struct
{
    int32_t temperature;
    uint32_t last_update;
    uint8_t node_id;
    uint8_t dc;    
    uint8_t priority;
} OtherBoilerStatus;


static int32_t target_power = 100;
static volatile pid_control_t pid_config;

static volatile uint8_t controller_dc;
static uint8_t boiler_dc;
static volatile uint8_t controller_update;
static volatile uint32_t controller_last_update;
static volatile int32_t max_boiler_temperature;
static volatile int32_t min_boiler_temperature;

/* temperature in centidegrees: 2500 is 25.00 degrees celsius */
static volatile int32_t boiler_temperature;

static volatile OtherBoilerStatus other_boiler_status[MAX_OTHER_BOILER_NODES];

static uint8_t calculate_priority(void)
{
    if(boiler_temperature > max_boiler_temperature) {
        return 0;
    }
    return 1;
}

static void send_status_message(uint8_t dc)
{
    uint8_t transferStatus;
    uint8_t buf[HOMEAUTOMATION_BOILERSTATUS_MAX_SIZE];
    homeautomation_BoilerStatus status;
    status.duty_cycle = dc;
    status.temperature = boiler_temperature;
    status.priority =  calculate_priority();
    homeautomation_BoilerStatus_encode(&status, buf);
    canardLockBroadcast(&canard,
    HOMEAUTOMATION_BOILERSTATUS_SIGNATURE,
    HOMEAUTOMATION_BOILERSTATUS_ID,
    &transferStatus,
    CANARD_TRANSFER_PRIORITY_LOW,
    buf,
    HOMEAUTOMATION_BOILERSTATUS_MAX_SIZE
    );
}


static void set_dc(uint8_t dc)
{
    boiler_dc = dc;
    dimmer_set_dc(dc);
    send_status_message(dc);
}

static void reconfigure(void)
{
    max_boiler_temperature = config_get_int(CONFIG_BOILER_MAX_TEMPERATURE);
    min_boiler_temperature = config_get_int(CONFIG_BOILER_MIN_TEMPERATURE);
    float kp = config_get_float(CONFIG_BOILER_PID_KP);
    float kd = config_get_float(CONFIG_BOILER_PID_KD);
    float ki = config_get_float(CONFIG_BOILER_PID_KI);
    pid_init(
        &pid_config,
        kp,
        kd,
        ki
        );
    target_power = config_get_int(CONFIG_BOILER_TARGET_POWER);
    dimmer_read_config();
}

void app_init(void)
{
    bme280_app_init();
    palSetPadMode(GPIOA, 0, PAL_MODE_INPUT_ANALOG); /* PA0: Temperature sensor */
    reconfigure();
}

/* calculates the thermistors temperature by given adc reading.
the temperature is returned in centidegrees. */
static int32_t calculate_thermistor_temperature(uint16_t adc)
{
    //1/T = 1/T0 + 1/B * ln( ( adcMax / adcVal ) – 1 )
    float inv_T0 = 1/298.15f;
    float inv_B = 1/4090.0f;
    float adcmax = ANALOG_MAX * 1.0f;
    float inv_t = qfp_fadd(inv_T0, qfp_fmul(inv_B, qfp_fln(qfp_fsub(qfp_fdiv(adcmax, qfp_uint2float(adc)), 1))));
    float t = qfp_fsub(qfp_fdiv(1, inv_t), 273.15f);
    return qfp_float2int(qfp_fmul(t, 100));
}

static uint8_t transform_dc(uint8_t dc)
{
    return dc;
}

void app_tick(void)
{
    bme280_app_read();
    boiler_temperature = calculate_thermistor_temperature(adc_smp_filtered[0]);
    /* turn off boiler when there is no obis message received */
    if(chVTTimeElapsedSinceX(controller_last_update) > TIME_S2I(CONTROLLER_TIMEOUT)) {
        node_debug(LOG_LEVEL_ERROR, "BOIL", "STOP boiler no meter reading");
        set_dc(0);
    }
}

void app_fast_tick(void)
{
    if(controller_update) {
        uint8_t target_dc = controller_dc;
        if((boiler_temperature < min_boiler_temperature) ||
           (boiler_dc == 100 && boiler_temperature < (min_boiler_temperature + 300)))
        {
            target_dc = 100;
        }
        if(boiler_temperature > max_boiler_temperature)
        {
            target_dc = 0;
        }
        target_dc = transform_dc(target_dc);
        set_dc(target_dc);
        controller_update = 0;
    }
}

void app_config_update(void)
{
    reconfigure();
}

static int32_t get_target_power(void)
{
    uint8_t i;
    /* offset of 2 so the result is dc when there is no other node and we have a balanced priority. */
    for(i = 0; i < MAX_OTHER_BOILER_NODES; ++i) {
        if(other_boiler_status[i].node_id) {
            if(chVTTimeElapsedSinceX(other_boiler_status[i].last_update) < TIME_S2I(OTHER_NODE_TIMEOUT)) {
                if(other_boiler_status[i].priority && other_boiler_status[i].temperature < boiler_temperature) {
                    /* there is another boiler that has lower temperature than us -> give it more power by regulating towards a lower power goal */
                    return 0;
                }
            } else {
                other_boiler_status[i].node_id = 0;
            }
        }
    }
    return target_power;
}

static void regulate(int32_t current_power, uint32_t timestamp)
{
    int32_t e = get_target_power() - current_power;
    int32_t result = pid_tick(&pid_config, e, timestamp);
    if(result < 0) {
        controller_dc = 0;
    } else if(result > 100)
    {
        controller_dc = 100;
    } else {
        controller_dc = result;
    }
    controller_last_update = timestamp;
    controller_update = 1;
}

void on_obis_data(CanardInstance* ins, CanardRxTransfer* transfer)
{
    static uint64_t last_180meter_reading;
    static uint32_t last_180meter_reading_ts;
    static uint64_t last_280meter_reading;
    static uint32_t last_280meter_reading_ts;
    static int32_t last180reading;
    static uint8_t fresh180reading;
    homeautomation_Obis message;
    homeautomation_Obis_decode(transfer, 0, &message, NULL);
    if(message.code[0] == 1 && message.code[1] == 8 && message.code[2] == 0) {
      /* total meter reading. Unit is 0.1 Wh */
        if(last_180meter_reading != 0) {
            int64_t difference = (message.value - last_180meter_reading);
            uint32_t time_difference = TIME_I2MS(transfer->timestamp - last_180meter_reading_ts);
            int32_t p_180 = qfp_float2int(qfp_fmul(qfp_fdiv(qfp_int2float(difference), time_difference), 360000));
            fresh180reading = 1;
            last180reading = p_180;
        }
        last_180meter_reading = message.value;
        last_180meter_reading_ts = transfer->timestamp;
    }
    if(message.code[0] == 2 && message.code[1] == 8 && message.code[2] == 0) {
      /* total meter reading. Unit is 0.1 Wh */
        if(last_280meter_reading != 0) {
            int64_t difference = (message.value - last_280meter_reading);
            uint32_t time_difference = TIME_I2MS(transfer->timestamp - last_280meter_reading_ts);
            int32_t p_280 = qfp_float2int(qfp_fmul(qfp_fdiv(qfp_int2float(difference), time_difference), 360000));
            if(fresh180reading) {
                regulate(last180reading - p_280, transfer->timestamp);
                fresh180reading = 0;
            } else {
                node_debug(LOG_LEVEL_WARNING, "BOIL", "received 280 without 180");
            }
            
        }
        last_280meter_reading = message.value;
        last_280meter_reading_ts = transfer->timestamp;
    }
    if(message.code[0] == 16 && message.code[1] == 7) {
        /* instantaneous power sum */
        int32_t current_power;
        if (message.value > 10000) {
            current_power = 10000;
        } else if (message.value < -2000) {
            current_power = -2000;
        } else {
            current_power = message.value;
        }
    }

}

static uint8_t get_other_boiler_entry(uint8_t node_id)
{
    uint8_t id;
    for(id = 0; id < MAX_OTHER_BOILER_NODES; ++id) {
        /* find first matching or unused entry. Entries are always filled up from beginning and _never_ removed */
        if(other_boiler_status[id].node_id == node_id || other_boiler_status[id].node_id == 0) {
            other_boiler_status[id].node_id = node_id;
            return id;
        }
    }
    /* todo: if no entry is found, overwrite the oldest one. We for now overwrite the first one. The tick method currently invalidates old entries as well */
    other_boiler_status[0].node_id = node_id;
    node_debug(LOG_LEVEL_ERROR, "BOIL", "No free boiler entry");
    return 0;
}

void on_boilerstatus_data(CanardInstance *ins, CanardRxTransfer* transfer)
{
    homeautomation_BoilerStatus status;
    OtherBoilerStatus *storedStatus;
    homeautomation_BoilerStatus_decode(transfer, 0, &status, NULL);

    storedStatus = &other_boiler_status[get_other_boiler_entry(transfer->source_node_id)];
    storedStatus->dc = status.duty_cycle;
    storedStatus->temperature = status.temperature;
    storedStatus->last_update = transfer->timestamp;
    storedStatus->priority = status.priority;
}
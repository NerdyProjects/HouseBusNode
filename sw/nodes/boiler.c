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
#include "modules/pid.h"
#include "config.h"
#include "chprintf.h"
#include "dimmer.h"
#include "drivers/analog.h"
#include "qfplib.h"

static int32_t target_power = 100;
static volatile pid_control_t pid_config;

static volatile uint8_t controller_dc;
static volatile uint8_t controller_update;

static void reconfigure(void)
{
    char dbgbuf[20];
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
    chsnprintf(dbgbuf, 20, "kp %d kd %d", qfp_float2int(qfp_fmul(kp, 1000)), qfp_float2int(qfp_fmul(kd, 1000)));
    node_debug(LOG_LEVEL_DEBUG, "BOIL", dbgbuf);
    chsnprintf(dbgbuf, 20, "ki %d tgt %d", qfp_float2int(qfp_fmul(ki, 1000)), target_power);
    node_debug(LOG_LEVEL_DEBUG, "BOIL", dbgbuf);
    dimmer_read_config();
}

void app_init(void)
{
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

void app_tick(void)
{
    char dbgbuf[20];
    chsnprintf(dbgbuf, 20, "dc %3d T %5d", controller_dc, calculate_thermistor_temperature(adc_smp_filtered[0]));
    node_debug(LOG_LEVEL_DEBUG, "BOIL", dbgbuf);
    /* TODO: turn off boiler when there is no obis message received */
}

void app_fast_tick(void)
{
    if(controller_update) {
        dimmer_set_dc(controller_dc);
        controller_update = 0;
    }
}

void app_config_update(void)
{
    reconfigure();
}

void on_obis_data(CanardInstance* ins, CanardRxTransfer* transfer)
{
    char dbgbuf[20];
    homeautomation_Obis message;
    homeautomation_Obis_decode(transfer, 0, &message, NULL);
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
        int32_t e = target_power - current_power;
        int32_t result = pid_tick(&pid_config, e, transfer->timestamp);
        chsnprintf(dbgbuf, 20, "p %d e %d", current_power, e);
        node_debug(LOG_LEVEL_DEBUG, "BOIL", dbgbuf);
        chsnprintf(dbgbuf, 20, "PID %d", result);
        node_debug(LOG_LEVEL_DEBUG, "BOIL", dbgbuf);
        if(result < 0) {
            controller_dc = 0;
        } else if(result > 100)
        {
            controller_dc = 100;
        } else {
            controller_dc = result;
        }
        controller_update = 1;
    }

}
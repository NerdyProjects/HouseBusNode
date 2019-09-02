#include <stdint.h>
#include "node.h"
#include "qfplib.h"
#include "pid.h"
#include "ch.h"
#include "chprintf.h"


void pid_init(pid_control_t *cfg, float kp, float kd, float ki) {
    cfg->kp = kp;
    cfg->kd = kd;
    cfg->ki = ki;
    cfg->first = 1;
    cfg->sum = 0;
}

int32_t pid_tick(pid_control_t *cfg, int32_t e, uint32_t timestamp) {
    //char dbgbuf[20];
    float ef = qfp_int2float(e);
    /* p part */
    float result = qfp_fmul(cfg->kp, ef);
    //chsnprintf(dbgbuf, 20, "PID-P: %d", qfp_float2int(qfp_fmul(result, 100)));
    //node_debug(LOG_LEVEL_DEBUG, "PIDt", dbgbuf);
    if(cfg->first == 0) {
        /* Time with ms precision */
        float time_diff = qfp_fdiv(qfp_int2float(TIME_I2MS(timestamp - cfg->last_timestamp)), 1000);
        /* d part */
        float d = qfp_fdiv(qfp_fmul(
            qfp_fsub(ef, cfg->last_ef),
            cfg->kd
        ), time_diff);
        result = qfp_fadd(result, d);
        //chsnprintf(dbgbuf, 20, "PID-D: %d", qfp_float2int(qfp_fmul(d, 100)));
        //node_debug(LOG_LEVEL_DEBUG, "PIDt", dbgbuf);
        /* i part */
        float di = qfp_fmul(qfp_fmul(ef, cfg->ki), time_diff);
        cfg->sum = qfp_fadd(cfg->sum, di);
        if(qfp_fcmp(cfg->sum, 100.0f) > 0) {
            cfg->sum = 100.0f;
        } else if(qfp_fcmp(0.0f, cfg->sum) > 0) {
            cfg->sum = 0.0f;
        }
        result = qfp_fadd(result, cfg->sum);
        //chsnprintf(dbgbuf, 20, "PID-I: %d", qfp_float2int(qfp_fmul(cfg->sum, 100)));
        //node_debug(LOG_LEVEL_DEBUG, "PIDt", dbgbuf);
    }
    cfg->last_timestamp = timestamp;
    cfg->last_ef = ef;
    cfg->first = 0;
    return qfp_float2int(result);
}

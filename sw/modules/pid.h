#include <stdint.h>

typedef struct {
    float kp;
    float kd;
    float ki;
    float last_ef;
    uint32_t last_timestamp;
    float sum;
    float sum_max;
    uint8_t first;
} pid_control_t;

void pid_init(pid_control_t *cfg, float kp, float kd, float ki);
int32_t pid_tick(pid_control_t *cfg, int32_t e, uint32_t timestamp);
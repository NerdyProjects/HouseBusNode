/*
 * pump_receiver.h
 */

#ifndef PUMP_RECEIVER_H_
#define PUMP_RECEIVER_H_

void pump_receiver_init(void);
void pump_receiver_tick(void);
bool pump_receiver_is_present(void);
void on_conduction_sensor_data(CanardRxTransfer* transfer);

#endif /* PUMP_RECEIVER_H_ */

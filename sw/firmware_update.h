/*
 * firmware_update.h
 *
 *  Created on: 24.01.2018
 *      Author: matthias
 */

#ifndef FIRMWARE_UPDATE_H_
#define FIRMWARE_UPDATE_H_

void onFileRead(CanardInstance* ins, CanardRxTransfer* transfer);
void processFirmwareUpdate(void);


#endif /* FIRMWARE_UPDATE_H_ */

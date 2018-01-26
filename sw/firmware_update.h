/*
 * firmware_update.h
 *
 *  Created on: 24.01.2018
 *      Author: matthias
 */

#ifndef FIRMWARE_UPDATE_H_
#define FIRMWARE_UPDATE_H_

#define FIRMWARE_UPDATE_ERR_FLASH_FAILED -2
#define FIRMWARE_UPDATE_DONE_SUCCESS 1
#define FIRMWARE_UPDATE_IN_PROGRESS 0

void onFileRead(CanardInstance* ins, CanardRxTransfer* transfer);
int processFirmwareUpdate(CanardInstance* ins);


#endif /* FIRMWARE_UPDATE_H_ */

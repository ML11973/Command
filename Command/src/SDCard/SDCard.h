/*
 * SDCard.h
 *
 * Created: 08.11.2017 11:21:12
 *  Author: voletqu
 */ 


#ifndef SDCARD_H_
#define SDCARD_H_

#include <asf.h>

void sdcard_Init(void);

bool sdcard_Mount(void);

bool sdcard_CheckPresence(void);

bool sdcard_ReadSector(void *ram, uint32_t sector);

#endif /* SDCARD_H_ */
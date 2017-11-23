/*
 * init.h
 *
 * Created: 06.11.2017 10:25:24
 *  Author: leemannma
 */ 

#ifndef INIT_H
#define INIT_H
#define TC1_CHANNEL 1
#define TC0_CHANNEL 0
/************************************************************************/
/* INCLUDES                                                             */
/************************************************************************/

#include <asf.h>
#include <board.h>
#include <conf_board.h>
#include <conf_cpu.h>
#include "audio/audio.h"
#include "SDCard/sdcard.h"
#include "RTC/rtc.h"



/************************************************************************/
/* PROTOTYPES	                                                        */
/************************************************************************/

void board_init(void);
void spi1_Init(void);
void tc1_Init(void);
void tc0_Init(void);
void twi_Init(void);
void usart_init(void);
void dac_Init(void);

#endif /* INIT_H */
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



/************************************************************************/
/* PROTOTYPES	                                                        */
/************************************************************************/

void board_init(void);
void spi_init(void);
void tc1_init(void);
void tc0_init(void);

#endif /* INIT_H */
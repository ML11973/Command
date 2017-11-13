/*
 * conf_cpu.h
 *
 * Created: 06.11.2017 10:18:34
 *  Author: leemannma
 */ 

#ifndef CONF_CPU_H
#define CONF_CPU_H

/************************************************************************/
/* DEFINITIONS                                                          */
/************************************************************************/

#define BOARD_OSC0_HZ 64000000
#define OSC0_STARTUP_US 3



/************************************************************************/
/* PROTOTYPES                                                           */
/************************************************************************/

void sysclk_init(void);

#endif /* CONF_CPU_H */
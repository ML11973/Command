/*
 * audio.h
 *
 * Created: 06.11.2017 13:52:58
 *  Author: leemannma
 */ 

#include <asf.h>

/************************************************************************/
/* DEFINITIONS                                                          */
/************************************************************************/

#define TC1_CHANNEL 1
#define TC0_CHANNEL 0


// DAC1 SPI address
#define DAC1 2

// Default volume
#define DEFAULTVOLUME 0x40

extern uint16_t sineTable[];
extern uint16_t i;
extern uint16_t j;
extern uint8_t max;
extern uint16_t audioL;
extern uint16_t audioR;

/************************************************************************/
/* PROTOTYPES                                                           */
/************************************************************************/

void audio_set_volume (uint8_t);
void audio_set_output (uint16_t, uint16_t);
void tc1_irq(void);
void tc0_irq(void);
void freq_start(uint16_t);
void freq_stop(void);
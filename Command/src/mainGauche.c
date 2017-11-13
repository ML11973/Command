/*
 * mainGauche.c
 *
 * Created: 10.11.2017 15:27:45
 *  Author: voletqu
 */ 

#ifndef MAIN_GAUCHE_C
#define MAIN_GAUCHE_C

#include <asf.h>

#include "audio/audio.h"
#include "SDCard/sdcard.h"

#include "mainGauche.h"

volatile uint32_t ram = 0;

void mainGauche(void){
	
	bool play = 0;
	
	board_init();
	sdcard_Init();
	sdcard_Mount();
	// Setting amplifier volume
	audio_set_volume(DEFAULTVOLUME);
	
	if(sdcard_CheckPresence()){
		play = 1;
	}
	else {
		play = 0;
	}
	
	
	while(1){
		if (gpio_get_pin_value(PIN_SWITCH0)){
			audio_set_volume(0);
			} else {
			audio_set_volume(DEFAULTVOLUME);
		}
		
		
		
		if (play){
			
		}
		
		
	}
}
#endif /* MAIN_GAUCHE_C */
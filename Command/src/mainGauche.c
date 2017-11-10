/*
 * mainGauche.c
 *
 * Created: 10.11.2017 15:27:45
 *  Author: voletqu
 */ 
#include <asf.h>

#include "audio/audio.h"

void mainGauche(void){
	board_init();
	freq_start (440);
	// Setting amplifier volume
	audio_set_volume(DEFAULTVOLUME);
	// CONFIRMED WORKING 08.11.17 MLN
	audio_set_output(0x7FF, 0x7FF);
	while(1){
		if (gpio_get_pin_value(PIN_SWITCH0)){
			audio_set_volume(0);
			} else {
			audio_set_volume(DEFAULTVOLUME);
		}
	}
}
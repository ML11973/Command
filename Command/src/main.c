/**
 * Command.c
 *
 * Command card software for Alarm project
 *
 */

/**
 * Main.c
 *
 * Contains only main function and interruptions. For configurations, check out
 * annex files.
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# "Insert system clock initialization code here" comment
 * -# Minimal main function that starts with a call to board_init()
 * -# "Insert application code here" comment
 *
 */

#include <asf.h>
#include <init.h>
#include "audio/audio.h"
#include "main.h"

int main (void) {
	
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



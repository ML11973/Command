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
#include "GFX/screen.h"
#include "GFX/gfx.h"
#include "GFX/gui.h"
#include "RTC/rtc.h"
#include "mainGauche.h"



void mainGauche(void){
	
	board_init();
	screen_Init();
	//sdcard_init();
	//gui_loadingScreen();
	screen_SetPixels(Rect(0,0,320,240), (Color){RED});
	
	currentTime.hours = 15;
	currentTime.minutes = 33;
	currentTime.seconds = 59;
	
	rtc_setTime();
	rtc_setMinutesInterrupt();
	
	gfx_BeginNewTerminal((Vector2){20,220});
	gfx_AddLineToTerminal("Debout, les damnes de la terre", 30, (Color){YELLOW}, 0);
	while(1){
		audio_playFile(0);
		if(timeChanged){
			if (currentTime.minutes > 59){
				currentTime.minutes = 0;
				if (currentTime.hours > 23){
					currentTime.hours = 0;
				}
				else {
					currentTime.hours++;
				}
			}
			else {
				currentTime.minutes++;
			}
			rtc_usart_sendTimeToDisplay();
			
			//currentTime.seconds = 59;
			//rtc_setTime();
			
			timeChanged = false;
		}
	}
	
}
#endif /* MAIN_GAUCHE_C */
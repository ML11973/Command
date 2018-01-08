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
	volatile uint8_t errorCode = 0;
	board_init();
	screen_Init();
	sdcard_init();
	
	rtc_setMinutesInterrupt();
	rtc_getTime();
	rtc_usart_sendTimeToDisplay();
	gui_Init();
	
	screen_SetPixels(Rect(0,0,320,240), (Color){RED});
	
	gfx_BeginNewTerminal((Vector2){20,220});
	gfx_AddLineToTerminal("Debout, les damnes de la terre", 30, (Color){YELLOW}, 0);
	
	errorCode = audio_setFileToPlay(0);	
			
	gfx_AddLineToTerminal(fileData.title, MAXMETADATACHARS, (Color){YELLOW}, 0);
	gfx_AddLineToTerminal(fileData.artist, MAXMETADATACHARS, (Color){YELLOW}, 0);
	gfx_AddLineToTerminal(fileData.album, MAXMETADATACHARS, (Color){YELLOW}, 0);
	gfx_AddLineToTerminal(fileData.creationYear, MAXMETADATACHARS, (Color){YELLOW}, 0);
	gfx_AddLineToTerminal(fileData.sDuration, MAXMETADATACHARS, (Color){YELLOW}, 0);
	
	
	gfx_AddLineToTerminal("Debout, les damnes de la terre", 30, (Color){YELLOW}, 0);
	
	while(1){
		audio_playFile();
		
		if(timeChanged){
			timeChanged = false;
			rtc_usart_sendTimeToDisplay();
			//currentTime.seconds = 59;
			//rtc_setTime();
		}
		if(alarmReached){
			alarmReached = false;
			rtc_setNextAlarm();
			//gfx_AddLineToTerminal("Debout, les damnes de la terre", 30, (Color){YELLOW}, 0);
			//currentTime.seconds = 59;
			//rtc_setTime();
		}
	}
	
}
#endif /* MAIN_GAUCHE_C */
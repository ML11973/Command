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
	
	currentTime.day = 1;
	currentTime.hours = 11;
	currentTime.minutes = 19;
	currentTime.seconds = 59;
	
	alarm[0].day = 1;
	alarm[0].hours = 11;
	alarm[0].minutes = 20;
	alarm[0].ringtoneIndex = 1;
	alarm[0].alarmEnable = true;
	
	alarm[1].day = 1;
	alarm[1].hours = 11;
	alarm[1].minutes = 21;
	alarm[1].ringtoneIndex = 0;
	alarm[1].alarmEnable = true;
	
	alarm[2].day = 2;
	alarm[2].hours = 19;
	alarm[2].minutes = 17;
	alarm[2].alarmEnable = true;
	
	alarm[3].day = 2;
	alarm[3].hours = 19;
	alarm[3].minutes = 16;
	alarm[3].alarmEnable = true;
	
	rtc_setTime();
	
	rtc_setNextAlarm();
		
	rtc_setMinutesInterrupt();
	
	rtc_usart_sendTimeToDisplay();
	
	gfx_BeginNewTerminal((Vector2){20,220});
	gfx_AddLineToTerminal("Debout, les damnes de la terre", 30, (Color){YELLOW}, 0);
	while(1){
		//audio_playFile(0);
		if(timeChanged){
			timeChanged = false;
			rtc_usart_sendTimeToDisplay();
			currentTime.seconds = 59;
			rtc_setTime();
		}
		if(alarmReached){
			alarmReached = false;
			rtc_setNextAlarm();
			gfx_AddLineToTerminal("Debout, les damnes de la terre", 30, (Color){YELLOW}, 0);
			currentTime.seconds = 59;
			rtc_setTime();
		}
	}
	
}
#endif /* MAIN_GAUCHE_C */
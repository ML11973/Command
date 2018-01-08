/*
 * mainCentre.c
 *
 * Created: 10.11.2017 15:28:32
 *  Author: voletqu
 */ 

#include <asf.h>
#include <init.h>

#include "mainCentre.h"
#include "SDCard/sdcard.h"
#include "GFX/screen.h"
#include "GFX/gui.h"


void mainCentre(void){
	board_init();
	
	screen_Init();
	//gui_loadingScreen();
	
	//cpu_delay_ms(500,BOARD_OSC0_HZ);
	rtc_setMinutesInterrupt();
	rtc_getTime();
	rtc_usart_sendTimeToDisplay();
	gui_Init();
	
	timeChanged = true;
	
	menus[currentMenuId](true);
	
	while(1){
		input[currentMenuId]();
		if(needRepaint || timeChanged){
			menus[currentMenuId](menuChanged);
			needRepaint = false;
			if(timeChanged){
				timeChanged = false;
				rtc_usart_sendTimeToDisplay();
			}
		}
		audio_playFile();
		
		
		if(alarmReached){
			alarmReached = false;
			audio_playFile(alarm[nextAlarmIndex].ringtoneIndex);
			rtc_setNextAlarm();
		}
	}
}
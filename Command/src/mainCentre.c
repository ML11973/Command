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
	rtc_usart_sendTimeToDisplay();
	gui_Init();
	
	menus[currentMenuId](true);
	
	while(1){
		input[currentMenuId]();
		if(needRepaint){
			menus[currentMenuId](menuChanged);
			needRepaint = false;
		}
		audio_playFile(0);
		
		if(timeChanged){
			timeChanged = false;
			rtc_usart_sendTimeToDisplay();
		}
		if(alarmReached){
			alarmReached = false;
			rtc_setNextAlarm();
		}
	}
}
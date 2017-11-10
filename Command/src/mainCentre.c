/*
 * mainCentre.c
 *
 * Created: 10.11.2017 15:28:32
 *  Author: voletqu
 */ 

#include <asf.h>
#include <init.h>

#include "SDCard/sdcard.h"
#include "GFX/screen.h"
#include "GFX/gui.h"

void mainCentre(void){
	sysclk_init();
	
	screen_Init();
	gui_loadingScreen();
	
	cpu_delay_ms(1500,BOARD_OSC0_HZ);
	
	sdcard_Init();
	gui_InitMenus();
	
	menus[0].draw(true);
	
	while(1){
		//menus[0].draw(false);
	}
}
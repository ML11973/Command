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
	sysclk_init();
	
	irq_initialize_vectors();
	cpu_irq_enable();
	
	screen_Init();
	gui_loadingScreen();
	
	cpu_delay_ms(500,BOARD_OSC0_HZ);
	
	sdcard_init();
	sdcard_mount();
	gui_Init();
	
	menus[currentMenuId](true);
	
	while(1){
		menus[currentMenuId](menuChanged);
	}
}
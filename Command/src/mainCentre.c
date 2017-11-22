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
#include "GFX/gfx.h"

uint8_t sectorsData[512];

void mainCentre(void){
	sysclk_init();
	
	irq_initialize_vectors();
	cpu_irq_enable();
	
	screen_Init();
	//gui_loadingScreen();
	
	//cpu_delay_ms(500,BOARD_OSC0_HZ);
	
	sdcard_init();
	sdcard_mount();
	//gui_Init();
	
	//menus[currentMenuId](true);
	
	sdcard_setFileToRead(0);
	
	while(1){/*
		input[currentMenuId]();
		if(needRepaint){
			menus[currentMenuId](menuChanged);
			needRepaint = false;
		}*/
	
		screen_SetPixels(Rect(0,0,320,240),(Color){BLACK});
		sdcard_getNextSectorFast(sectorsData);
		
		gfx_BeginNewTerminal((Vector2){0,200});
		for(uint8_t i = 0; i < 16; i++){
			gfx_AddLineToTerminal((char*)(sectorsData + (i * 32)), 32, (Color){GREEN}, false);	
		}
		
		cpu_delay_ms(100, BOARD_OSC0_HZ);
		/*
		screen_SetPixels(Rect(0,0,320,240),(Color){BLACK});
		gfx_BeginNewTerminal((Vector2){0,200});
		
		for(uint8_t i = 8; i < 16; i++){
			gfx_AddLineToTerminal((char*)(sectorsData + (i * 32)), 32, (Color){GREEN}, false);
		}*/
	}
}
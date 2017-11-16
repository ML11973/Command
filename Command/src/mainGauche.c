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

//volatile uint32_t ram = 0;

void mainGauche(void){
	
	board_init();
	screen_Init();
	gui_loadingScreen();
	screen_SetPixels(Rect(0,0,320,240), (Color){BLACK});
	
	
	gfx_BeginNewTerminal((Vector2){20,220});
	gfx_AddLineToTerminal("Heure : ", 8, (Color){WHITE}, 0);
	while(1){
		
		rtc_getTime();
		
		cpu_delay_ms(1000, BOARD_OSC0_HZ);
	}
	
}
#endif /* MAIN_GAUCHE_C */
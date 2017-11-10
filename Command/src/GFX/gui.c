/*
 * gui.c
 *
 * Created: 10.11.2017 11:19:45
 *  Author: QVT
 */ 


/************************************************************************/
/* INCLUDES                                                             */
/************************************************************************/

#include <asf.h>

#include "gui.h"
#include "GFX/gfx.h"


/************************************************************************/
/* DEFINITIONS                                                          */
/************************************************************************/

#define NBR_OF_MENU 1

#define MAINMENU_ID 0

/************************************************************************/
/* VARIABLES                                                            */
/************************************************************************/

Menu *menus;

/************************************************************************/
/* FUNCTIONS                                                            */
/************************************************************************/

void mainMenu(bool firstDraw);

bool gui_InitMenus(){
	menus = (Menu *) malloc(NBR_OF_MENU * sizeof(Menu));
	
	if(menus == NULL){
		screen_SetPixels(Rect(0,0,320,240),(Color){BLACK});
		gfx_Label((Vector2){120,120},"gui_InitMenus();",16,Small, (Color){RED});
		gfx_Label((Vector2){110,120},"malloc failed",16,Small, (Color){RED});
		return false;
	}
	
	for(uint8_t n = 0; n < NBR_OF_MENU; n++){
		menus[n].draw = mainMenu;
		menus[n].input = NULL;
	}
	return true;
}

void gui_loadingScreen(void){
	screen_SetPixels(Rect(0,0,320,240), (Color){RED});
	screen_SetPixels((Rectangle){.bottomLeft={10,10},.topRight={310,230}}, (Color){BLACK});

	gfx_DrawCircle((Vector2){150,129},58,1,(Color){WHITE});
	gfx_DrawCircle((Vector2){200,100},50,50,(Color){RED});
	gfx_DrawLine((Vector2){100,100}, (Vector2){200,100}, (Color){WHITE}, 3);
	gfx_DrawLine((Vector2){150, 186}, (Vector2){200,100}, (Color){WHITE}, 1);
	gfx_DrawCircle((Vector2){100,100},40,5,(Color){RED});
	gfx_DrawLine((Vector2){99,99}, (Vector2){150, 186}, (Color){WHITE}, 15);
	char text[] = "LogoRetro.pixel";
	gfx_Label((Vector2){40,40}, text, sizeof(text)/sizeof(*text), Small, (Color){WHITE});
}

/* mainMenu
 *
 * Draw the main menu, if firstDraw is true, all the menu is drawn 
 * otherwise only dynamic part are drawn
 *
 * Created 10.11.17 QVT
 * Last modified 10.11.17 QVT
 */
void mainMenu(bool firstDraw){
	if(firstDraw){
		screen_SetPixels(Rect(0,0,320,240),(Color){BLACK});
	}
	
	//Shortcut Highlight
	screen_SetPixels(Rect(2,2,12,14),(Color){WHITE});
	screen_SetPixels(Rect(80,2,92,14),(Color){WHITE});
	screen_SetPixels(Rect(158,2,170,14),(Color){WHITE});
	screen_SetPixels(Rect(236,2,248,14),(Color){WHITE});
	//Shortcut
	gfx_Label((Vector2){4,4},"<", 7, Small, (Color){BLACK});
	gfx_Label((Vector2){82,4},"v", 7, Small, (Color){BLACK});
	gfx_Label((Vector2){160,4},"^", 7, Small, (Color){BLACK});
	gfx_Label((Vector2){238,4},"!", 7, Small, (Color){BLACK});
	//Button Description
	gfx_Label((Vector2){14,4},"Back", 7, Small, (Color){WHITE});
	gfx_Label((Vector2){94,4},"Down", 7, Small, (Color){WHITE});
	gfx_Label((Vector2){172,4},"Up", 7, Small, (Color){WHITE});
	gfx_Label((Vector2){250,4},"Ok", 7, Small, (Color){WHITE});
		
	//Time
	gfx_Label((Vector2){20, 240 - 20}, "status --complete", 17, Small, (Color){WHITE});
	cpu_delay_ms(100,BOARD_OSC0_HZ);
	gfx_Label((Vector2){20, 240 - 40}, "> Vendredi", 10, Small, (Color){WHITE});
	cpu_delay_ms(100,BOARD_OSC0_HZ);
	gfx_Label((Vector2){20, 240 - 60}, "> 10/11/17", 10, Small, (Color){WHITE});
	cpu_delay_ms(100,BOARD_OSC0_HZ);
	gfx_Label((Vector2){20, 240 - 80}, "> 15h 30m" ,  9, Small, (Color){WHITE});
	cpu_delay_ms(100,BOARD_OSC0_HZ);
	gfx_Label((Vector2){20, 240 - 100}, "> Alarme 1 disabled" ,  28, Small, (Color){WHITE});
	cpu_delay_ms(100,BOARD_OSC0_HZ);
	gfx_Label((Vector2){20, 240 - 120}, "> Alarme 2 enabled" ,  27, Small, (Color){WHITE});
}
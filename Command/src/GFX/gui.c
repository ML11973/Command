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

#include "GFX/gui.h"
#include "GFX/gfx.h"
#include "SDCard/sdcard.h"


/************************************************************************/
/* DEFINITIONS                                                          */
/************************************************************************/

#define NBR_OF_MENU 1

#define MAINMENU_ID 0

/************************************************************************/
/* TYPEDEF                                                              */
/************************************************************************/

typedef enum menu_id{
	MAIN,
	MUSIC,
	SETTINGS
}MENU_ID;

/************************************************************************/
/* GUI FUNCTIONS PROTOTYPES                                             */
/************************************************************************/

void mainMenu(bool firstDraw);
void musicMenu(bool firstDraw);

__attribute__((__interrupt__)) void switchISR(void);

/************************************************************************/
/* VARIABLES                                                            */
/************************************************************************/

guiMenu menus = {mainMenu, musicMenu};
	
Color textColor = {GREEN};
	
bool menuChanged = false;
uint8_t currentMenuId = 0;

static uint8_t switchState = 0;

/************************************************************************/
/* FUNCTIONS                                                            */
/************************************************************************/

void gui_Init(void){
	gpio_enable_pin_glitch_filter(PIN_SWITCH0);
	gpio_enable_pin_pull_up(PIN_SWITCH0);
	gpio_enable_pin_interrupt(PIN_SWITCH0, GPIO_RISING_EDGE);

	gpio_enable_pin_glitch_filter(PIN_SWITCH1);
	gpio_enable_pin_pull_up(PIN_SWITCH1);
	gpio_enable_pin_interrupt(PIN_SWITCH1, GPIO_RISING_EDGE);

	gpio_enable_pin_glitch_filter(PIN_SWITCH2);
	gpio_enable_pin_pull_up(PIN_SWITCH2);
	gpio_enable_pin_interrupt(PIN_SWITCH2, GPIO_RISING_EDGE);

	gpio_enable_pin_glitch_filter(PIN_SWITCH3);
	gpio_enable_pin_pull_up(PIN_SWITCH3);
	gpio_enable_pin_interrupt(PIN_SWITCH3, GPIO_RISING_EDGE);
	
	INTC_register_interrupt(&switchISR, AVR32_GPIO_IRQ7, AVR32_INTC_INT0);
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
	
		gfx_DrawTerminalButton((Vector2){4,4},"<","Back",4,textColor);
		gfx_DrawTerminalButton((Vector2){82,4},"v","Down",4,textColor);
		gfx_DrawTerminalButton((Vector2){160,4},"^","Up",4,textColor);
		gfx_DrawTerminalButton((Vector2){238,4},">","Ok",4,textColor);
		menuChanged = false;
	}
	
	gfx_BeginNewTerminal((Vector2){20,220});
		
	gfx_AddLineToTerminal("status --complete", 17, textColor);
	gfx_AddLineToTerminal("> Vendredi", 10, textColor);
	gfx_AddLineToTerminal("> 10/11/17", 10, textColor);
	gfx_AddLineToTerminal("> 15h 30m" ,  9, textColor);
	gfx_AddLineToTerminal("> Alarme 1 disabled" ,  28, textColor);
	gfx_AddLineToTerminal("> Alarme 2 enabled" ,  27, textColor);
	
	if(switchState == 4){
		currentMenuId = MUSIC;
		menuChanged = true;
		switchState = 0;
	}
}

/* mainMenu
 *
 * Draw the main menu, if firstDraw is true, all the menu is drawn 
 * otherwise only dynamic part are drawn
 *
 * Created 10.11.17 QVT
 * Last modified 10.11.17 QVT
 */
void musicMenu(bool firstDraw){
	if(firstDraw){
		screen_SetPixels(Rect(0,0,320,240),(Color){BLACK});
	
		gfx_DrawTerminalButton((Vector2){4,4},"<","Back",4,textColor);
		gfx_DrawTerminalButton((Vector2){82,4},"v","Down",4,textColor);
		gfx_DrawTerminalButton((Vector2){160,4},"^","Up",4,textColor);
		gfx_DrawTerminalButton((Vector2){238,4},">","Ok",4,textColor);
		menuChanged = false;
	}
	
	gfx_BeginNewTerminal((Vector2){20,220});
		
	gfx_AddLineToTerminal("ls | grep .wave", 15, textColor);
	if(sdcard_CheckPresence()){
		gfx_AddLineToTerminal((char*)(file_menu[0].name),25,textColor);
	}
	else{
		gfx_AddLineToTerminal("No results",10,(Color){RED});
	}
	
	if(switchState == 1){
		currentMenuId = MAIN;
		menuChanged = true;
		switchState = 0;
	}
}

__attribute__((__interrupt__)) void switchISR(void){
	if 	(gpio_get_pin_interrupt_flag(PIN_SWITCH0))	{
		switchState = 1;
	}
	else if (gpio_get_pin_interrupt_flag(PIN_SWITCH1)){
		switchState = 2;
	}
	else if (gpio_get_pin_interrupt_flag(PIN_SWITCH2)){
		switchState = 3;
	}
	else if (gpio_get_pin_interrupt_flag(PIN_SWITCH3)){
		switchState = 4;
	}
	AVR32_GPIO.port[1].ifrc = 0xFF000000;
}
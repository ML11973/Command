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

#define NBR_OF_MENU 3



/************************************************************************/
/* TYPEDEF                                                              */
/************************************************************************/

typedef enum menu_id{
	MAIN,
	MUSIC,
	SETTINGS,
	ALARM
}MENU_ID;

/************************************************************************/
/* GUI FUNCTIONS PROTOTYPES                                             */
/************************************************************************/

void _mainMenu(bool firstDraw);
void _musicMenu(bool firstDraw);
void _settingsMenu(bool firstDraw);
void _alarmMenu(bool firstDraw);

__attribute__((__interrupt__)) void switchISR(void);

/************************************************************************/
/* VARIABLES                                                            */
/************************************************************************/

guiMenu menus = {_mainMenu, _musicMenu, _settingsMenu, _alarmMenu};
	
Color textColor = {GREEN};
	
bool menuChanged = false;
uint8_t currentMenuId = 0;

static uint8_t switchState = 0;
static uint8_t selectedCommand = 0;
static bool commandChanged = true;
static uint8_t compteur = 0;
static uint8_t selectedOption = 1;

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
	screen_SetPixels(Rect(0,0,320,240), textColor);
	screen_SetPixels((Rectangle){.bottomLeft={10,10},.topRight={310,230}}, (Color){BLACK});

	gfx_DrawCircle((Vector2){150,129},58,1,(Color){WHITE});
	gfx_DrawCircle((Vector2){200,100},50,50,textColor);
	gfx_DrawLine((Vector2){100,100}, (Vector2){200,100}, (Color){WHITE}, 3);
	gfx_DrawLine((Vector2){150, 186}, (Vector2){200,100}, (Color){WHITE}, 1);
	gfx_DrawCircle((Vector2){100,100},40,5,textColor);
	gfx_DrawLine((Vector2){99,99}, (Vector2){150, 186}, (Color){WHITE}, 15);
	char text[] = "LogoRetro.pixel";
	gfx_Label((Vector2){40,40}, text, sizeof(text)/sizeof(*text), Small, (Color){WHITE});
}

void _drawButton(void){
	gfx_DrawTerminalButton((Vector2){4,4},"<","Back",4,textColor);
	gfx_DrawTerminalButton((Vector2){82,4},"^","Up",4,textColor);
	gfx_DrawTerminalButton((Vector2){160,4},"v","Down",4,textColor);
	gfx_DrawTerminalButton((Vector2){238,4},">","Ok",4,textColor);
}

/* mainMenu
 *
 * Draw the main menu, if firstDraw is true, all the menu is drawn 
 * otherwise only dynamic part are drawn
 *
 * Created 10.11.17 QVT
 * Last modified 10.11.17 QVT
 */
void _mainMenu(bool firstDraw){
	if(firstDraw){
		screen_SetPixels(Rect(0,0,320,240),(Color){BLACK});
	
		_drawButton();
		menuChanged = false;
		
		gfx_BeginNewTerminal((Vector2){20,220});
		
		gfx_AddLineToTerminal("status --complete", 17, textColor, firstDraw);
	}
	
	gfx_BeginNewTerminal((Vector2){20,200});
	gfx_AddOptionToTerminal("Vendredi",8,textColor,false,firstDraw,firstDraw);
	gfx_AddOptionToTerminal("10/11/17", 8, textColor,false,firstDraw, firstDraw);
	gfx_AddOptionToTerminal("15h 30m" ,  7, textColor,false,firstDraw, firstDraw);
	gfx_AddOptionToTerminal("Alarme 1 disabled" ,  26, textColor,false,firstDraw, firstDraw);
	gfx_AddOptionToTerminal("Alarme 2 enabled" ,  25, textColor,false,firstDraw, firstDraw);
	
	if(selectedCommand == 0){
		gfx_cmdLine("ls | grep .wave", 15, textColor,commandChanged);
		commandChanged = false;
	}
	else if(selectedCommand == 1){
		gfx_cmdLine("sudo settings -h", 16, textColor,commandChanged);
		commandChanged = false;
	}
	else {
		gfx_cmdLine("alarm -info", 11, textColor,commandChanged);
		commandChanged = false;
	}
	
	if(switchState == 4){
		if(selectedCommand == 0)
			currentMenuId = MUSIC;
		else if(selectedCommand == 1)
			currentMenuId = SETTINGS;
		else
			currentMenuId = ALARM;
			
		menuChanged = true;
		switchState = 0;
	}
	else if(switchState == 2){
		selectedCommand = (selectedCommand == NBR_OF_MENU - 1)?(0):(selectedCommand+1);
		commandChanged = true;
		screen_SetPixels(Rect(0,40,320,50),(Color){BLACK});
		switchState = 0;
	}
	else if(switchState == 3){
		selectedCommand = (selectedCommand == 0)?(NBR_OF_MENU - 1):(selectedCommand-1);
		commandChanged = true;
		screen_SetPixels(Rect(0,40,320,50),(Color){BLACK});
		switchState = 0;
	}
}

/* mainMenu
 *
 * Draw the music menu, if firstDraw is true, all the menu is drawn 
 * otherwise only dynamic part are drawn
 *
 * Created 13.11.17 QVT
 * Last modified 13.11.17 QVT
 */
void _musicMenu(bool firstDraw){
	if(firstDraw){
		screen_SetPixels(Rect(0,0,320,240),(Color){BLACK});
	
		_drawButton();
		menuChanged = false;
	}
	
	gfx_BeginNewTerminal((Vector2){20,220});
		
	gfx_AddLineToTerminal("ls | grep .wave", 15, textColor, firstDraw);
	
	if(sdcard_checkPresence()){
		for(uint8_t i = 0; i < 6; i++){
			gfx_AddLineToTerminal((char*)(files[i].name),25,textColor, firstDraw);
		}
	}
	else{
		gfx_AddLineToTerminal("> No results",12,(Color){RED}, firstDraw);
	}
	
	if(switchState == 1){
		currentMenuId = MAIN;
		menuChanged = true;
		commandChanged = true;
		switchState = 0;
	}
}

void _settingsMenu(bool firstDraw){
	if(firstDraw){
		screen_SetPixels(Rect(0,0,320,240),(Color){BLACK});
		
		_drawButton();
		menuChanged = false;
		
		gfx_BeginNewTerminal((Vector2){20,220});
		gfx_AddLineToTerminal("sudo settings -h", 16, textColor, firstDraw);
		gfx_AddLineToTerminal("> textColor = Green", 20, textColor, firstDraw);
		gfx_AddLineToTerminal("> Heure : 10h 30m", 18, textColor, firstDraw);
		gfx_AddLineToTerminal("> Date : 09h 35m", 17, textColor, firstDraw);
	}
	
	if(switchState == 1){
		currentMenuId = MAIN;
		menuChanged = true;
		commandChanged = true;
		switchState = 0;
	}
}

void _alarmMenu(bool firstDraw){
	if(firstDraw){
		screen_SetPixels(Rect(0,0,320,240),(Color){BLACK});
		
		_drawButton();
		menuChanged = false;
		
		gfx_BeginNewTerminal((Vector2){20,220});
		gfx_AddLineToTerminal("alarme -info", 12, textColor, firstDraw);
		
	}

	gfx_BeginNewTerminal((Vector2){20,200});
	gfx_AddOptionToTerminal("Alarme 1 : 18h 00m", 18,textColor, selectedOption & (1<<0), firstDraw, firstDraw);
	gfx_AddOptionToTerminal("Alarme 2 : 09h 35m", 18,textColor, selectedOption & (1<<1), firstDraw, firstDraw);
	gfx_AddOptionToTerminal("Alarme 3 : 12h 00m", 18,textColor, selectedOption & (1<<2), firstDraw, firstDraw);
	gfx_AddOptionToTerminal("Alarme 4 : 06h 18m", 18,textColor, selectedOption & (1<<3), firstDraw, firstDraw);
	
	if(switchState == 1){
		currentMenuId = MAIN;
		menuChanged = true;
		commandChanged = true;
		switchState = 0;
	}
	else if(switchState == 2){
		(selectedOption &(1<<0))?(selectedOption = (1<<3)):(selectedOption>>=1);
		switchState = 0;
	}
	else if (switchState == 3){
		(selectedOption &(1<<3))?(selectedOption = (1<<0)):(selectedOption<<=1);
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
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
#include "RTC/rtc.h"
#include "audio/audio.h"

/************************************************************************/
/* DEFINITIONS                                                          */
/************************************************************************/

#define NBR_OF_MENU 5



/************************************************************************/
/* TYPEDEF                                                              */
/************************************************************************/

typedef enum menu_id{
	MAIN,
	MUSIC,
	SETTINGS,
	ALARM,
	MUSIC_PLAYER,
	SETTINGS_HOUR
}MENU_ID;



/************************************************************************/
/* FUNCTIONS PROTOTYPES					                                */
/************************************************************************/

void _mainMenu(bool firstDraw);
void _musicMenu(bool firstDraw);
void _settingsMenu(bool firstDraw);
void _alarmMenu(bool firstDraw);
void _musicPlayerMenu(bool firstDraw);
void _settingsHourMenu(bool firstDraw);

void _mainInput(void);
void _musicInput(void);
void _settingsInput(void);
void _alarmInput(void);
void _musicPlayerInput(void);
void _settingsHourInput(void);

void _playFile(uint8_t fileNbr);

__attribute__((__interrupt__)) void switch_irq(void);



/************************************************************************/
/* VARIABLES                                                            */
/************************************************************************/

guiMenu	menus[NBR_OF_MENU + 1]	= {
	_mainMenu, 
	_musicMenu,
	_settingsMenu,
	_alarmMenu, 
	_musicPlayerMenu,
	_settingsHourMenu
};

inputHandler input[NBR_OF_MENU + 1]	= {
	_mainInput, 
	_musicInput, 
	_settingsInput, 
	_alarmInput, 
	_musicPlayerInput,
	_settingsHourInput
};

inputBinding binding[NBR_OF_MENU + 1][2] = {{NULL},{_playFile}};	

Color textColor = {GREEN};
	
bool menuChanged = false;
bool needRepaint = false;
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
	
	INTC_register_interrupt(&switch_irq, AVR32_GPIO_IRQ7, AVR32_INTC_INT0);
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

void _drawCommonButton(void){
	gfx_DrawTerminalButton((Vector2){4,4},"<","Back",4,textColor);
	gfx_DrawTerminalButton((Vector2){82,4},"^","Up",4,textColor);
	gfx_DrawTerminalButton((Vector2){160,4},"v","Down",4,textColor);
	gfx_DrawTerminalButton((Vector2){238,4},">","Ok",4,textColor);
}

void _drawMusicButton(void){
	gfx_DrawTerminalButton((Vector2){4,4},"X","Back",4,textColor);
	gfx_DrawTerminalButton((Vector2){82,4},"<","VolDwn",7,textColor);
	gfx_DrawTerminalButton((Vector2){160,4},">","VolUp",5,textColor);
	gfx_DrawTerminalButton((Vector2){238,4},"P","Pause",5,textColor);
}

void _drawSettingsButton(void){
	gfx_DrawTerminalButton((Vector2){4,4},"<","Back",4,textColor);
	gfx_DrawTerminalButton((Vector2){82,4},"+","Plus",4,textColor);
	gfx_DrawTerminalButton((Vector2){160,4},"-","Minus",5,textColor);
	gfx_DrawTerminalButton((Vector2){238,4},"v","OK",2,textColor);
}
/************************************************************************/
/* MENU FUNCTIONS                                                       */
/************************************************************************/
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
	
		_drawCommonButton();
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
	static bool wasSDCardConnected;
	if(firstDraw){
		selectedOption = 1;
		wasSDCardConnected = false;
		if(sdcard_checkPresence()){
			sdcard_mount();
		}
		screen_SetPixels(Rect(0,0,320,240),(Color){BLACK});
	
		_drawCommonButton();
		menuChanged = false;
	
		gfx_BeginNewTerminal((Vector2){20,220});
		
		gfx_AddLineToTerminal("ls | grep .wav", 14, textColor, firstDraw);
	}
	
	gfx_BeginNewTerminal((Vector2){20,200});
		
	if(wasSDCardConnected != sdcard_checkPresence()){
		screen_SetPixels(Rect(20,14,320,220),(Color){BLACK});
		wasSDCardConnected = sdcard_checkPresence();
		if(!wasSDCardConnected){
			gfx_AddLineToTerminal("> No results",12,(Color){RED}, firstDraw);
		}
	}
	if(wasSDCardConnected){
		for(uint8_t i = 0; i < min(6,nbrOfFiles); i++){
			gfx_AddOptionToTerminal((char*)(files[i].name),25,textColor, selectedOption & (1<<(i)), firstDraw, firstDraw);
		}
	}
	
}

void _settingsMenu(bool firstDraw){
	if(firstDraw){
		screen_SetPixels(Rect(0,0,320,240),(Color){BLACK});
		
		_drawCommonButton();
		menuChanged = false;
		
		gfx_BeginNewTerminal((Vector2){20,220});
		gfx_AddLineToTerminal("sudo settings -h", 16, textColor, firstDraw);
		gfx_AddLineToTerminal("> textColor = Green", 20, textColor, firstDraw);
		gfx_AddLineToTerminal("> Heure : 10h 30m", 18, textColor, firstDraw);
		gfx_AddLineToTerminal("> Date : 09h 35m", 17, textColor, firstDraw);
	}
}

void _alarmMenu(bool firstDraw){
	if(firstDraw){
		screen_SetPixels(Rect(0,0,320,240),(Color){BLACK});
		
		_drawCommonButton();
		menuChanged = false;
		
		gfx_BeginNewTerminal((Vector2){20,220});
		gfx_AddLineToTerminal("alarme -info", 12, textColor, firstDraw);
		
	}

	gfx_BeginNewTerminal((Vector2){20,200});
	gfx_AddOptionToTerminal("Alarme 1 : 18h 00m", 18,textColor, selectedOption & (1<<0), firstDraw, firstDraw);
	gfx_AddOptionToTerminal("Alarme 2 : 09h 35m", 18,textColor, selectedOption & (1<<1), firstDraw, firstDraw);
	gfx_AddOptionToTerminal("Alarme 3 : 12h 00m", 18,textColor, selectedOption & (1<<2), firstDraw, firstDraw);
	gfx_AddOptionToTerminal("Alarme 4 : 06h 18m", 18,textColor, selectedOption & (1<<3), firstDraw, firstDraw);
}

void _musicPlayerMenu(bool firstDraw){
	if(firstDraw){
		screen_SetPixels(Rect(0,0,320,240),(Color){BLACK});
			
		_drawMusicButton();
			
		gfx_BeginNewTerminal((Vector2){20,220});
		gfx_AddLineToTerminal("play *.wav", 10, textColor, firstDraw);
		
		menuChanged = false;
		audio_togglePausePlay();
	}
}

void _settingsHourMenu(bool firstDraw){
	if(firstDraw){
		screen_SetPixels(Rect(0,0,320,240),(Color){BLACK});
		
		_drawSettingsButton();
		
		gfx_BeginNewTerminal((Vector2){20,220});
		gfx_AddLineToTerminal("time -set -hours", 16, textColor, firstDraw);
		
		menuChanged = false;
		
		gfx_BeginNewTerminal((Vector2){40,200});
		gfx_AddLineToTerminal("Hour = ",7,textColor, true);
		gfx_AddLineToTerminal("Minute = ",9,textColor, true);
	}
	
	uint8_t h[2] = {currentTime.hours/10 + 48, currentTime.hours % 10 + 48};
	uint8_t m[2] = {currentTime.minutes/10 + 48, currentTime.minutes % 10 + 48};
		
	screen_SetPixels(Rect(40 + 7*10, 200, 40 + 9*10, 220),(Color){BLACK});
	gfx_Label((Vector2){40 + 7 * 10, 200},(char*)h, 2, Small, textColor);
	//gfx_Label((Vector2){40 + 7 * 10, 200},(char*)((currentTime.hours/10) + 48), 1, Small, textColor);
	//gfx_Label((Vector2){40 + 8 * 10, 200},(char*)((currentTime.hours%10) + 48), 1, Small, textColor);
		
	screen_SetPixels(Rect(40 + 9*10, 180, 40 + 11*10, 200),(Color){BLACK});
	gfx_Label((Vector2){40 + 9 * 10, 180},(char*)m, 2, Small, textColor);
	//gfx_Label((Vector2){40 + 9 * 10, 180},(char*)(currentTime.minutes/10 + 48), 1, Small, textColor);
	//gfx_Label((Vector2){40 + 10 * 10, 180},(char*)(currentTime.minutes%10 + 48), 1, Small, textColor);
}

/************************************************************************/
/* INPUT FUNCTIONS                                                      */
/************************************************************************/

/* _mainInput
 *
 * Handle input for the main menu
 *
 * Created 20.11.17 QVT
 * Last modified 20.11.17 QVT
 */
void _mainInput(void){
	if(switchState == 0)
		return;
	needRepaint = true;
	
	if(switchState == 4){
		if(selectedCommand == 0)
			currentMenuId = MUSIC;
		else if(selectedCommand == 1)
			currentMenuId = SETTINGS;
		else
			currentMenuId = ALARM;
			
		menuChanged = true;
	}
	else if(switchState == 2){
		selectedCommand = (selectedCommand == NBR_OF_MENU - 1)?(0):(selectedCommand+1);
		commandChanged = true;
		screen_SetPixels(Rect(0,40,320,50),(Color){BLACK});
	}
	else if(switchState == 3){
		selectedCommand = (selectedCommand == 0)?(NBR_OF_MENU - 1):(selectedCommand-1);
		commandChanged = true;
		screen_SetPixels(Rect(0,40,320,50),(Color){BLACK});
	}
	switchState = 0;
}

/* _musicInput
 *
 * Handle input for the musics menu
 *
 * Created 20.11.17 QVT
 * Last modified 20.11.17 QVT
 */
void _musicInput(void){
	if(switchState == 0)
		return;
	needRepaint = true;
	
	if(switchState == 1){
		currentMenuId = MAIN;
		menuChanged = true;
		commandChanged = true;
	}
	else if(switchState == 2){
		(selectedOption &(1<<0))?(selectedOption = (1<<(min(6,nbrOfFiles)-1))):(selectedOption>>=1);
	}
	else if (switchState == 3){
		(selectedOption &(1<<(min(6,nbrOfFiles)-1)))?(selectedOption = (1<<0)):(selectedOption<<=1);
	}else if(switchState == 4){
		binding[MUSIC][0](selectedOption);
		currentMenuId = MUSIC_PLAYER;
		menuChanged = true;
	}
	switchState = 0;
}

/* _settingsInput
 *
 * Handle input for the settings menu
 *
 * Created 20.11.17 QVT
 * Last modified 20.11.17 QVT
 */
void _settingsInput(void)	{
	if(switchState == 0)
		return;
	needRepaint = true;
	
	if(switchState == 1){
		currentMenuId = MAIN;
		menuChanged = true;
		commandChanged = true;
	}
	else if(switchState == 4){
		currentMenuId = SETTINGS_HOUR;
		menuChanged = true;
	}
	switchState = 0;
}

/* _alarmInput
 *
 * Handle input for the alarms menu
 *
 * Created 20.11.17 QVT
 * Last modified 20.11.17 QVT
 */
void _alarmInput(void){
	if(switchState == 0)
		return;
	needRepaint = true;
	
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

void _musicPlayerInput(void){
	if(switchState == 0)
		return;
	needRepaint = true;
		
	if(switchState == 1){
		//stop audio and return to previous menu
		audio_stop();
		currentMenuId = MUSIC;
		menuChanged = true;
	}
	else if(switchState == 2){
		//volume down
		audio_setVolume(currentVolume - 10);
	}
	else if(switchState == 3){
		//volume up
		audio_setVolume(currentVolume + 10);
	}
	else if(switchState == 4){
		//toggle between pause and play
		audio_togglePausePlay();
		
	}
	switchState = 0;
}

void _settingsHourInput(void){
	if(switchState == 0)
		return;
	needRepaint = true;
	
	if(switchState == 1){
		currentMenuId = SETTINGS;
		menuChanged = true;
	}
	else if(switchState == 2){
		switch(selectedOption){
			case 1:
				currentTime.hours++;
				break;
			case 2:
				currentTime.minutes++;
				break;
			/*case 3:
				currentTime.seconds++;
				break;*/
		}
	}
	else if(switchState == 3){
		switch(selectedOption){
			case 1:
				currentTime.hours--;
				break;
			case 2:
				currentTime.minutes--;
				break;
			/*case 3:
				currentTime.seconds--;
				break;*/
		}
	}
	else if(switchState == 4){
		selectedOption++;
		
		//Hour, minutes /*and seconds*/ are set
		if(selectedOption > 2){
			currentMenuId = SETTINGS;
			menuChanged = true;
		}
	}
	switchState = 0;
}

/************************************************************************/
/* BINDING FUNCTIONS                                                    */
/************************************************************************/

void _playFile(uint8_t fileNbr){
	for(uint8_t i = 0; i < min(6,nbrOfFiles); i++){
		if(fileNbr & (1<<i)){
			audio_setFileToPlay(i);
			audio_togglePausePlay();
		}
	}
}

__attribute__((__interrupt__)) void switch_irq(void){
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
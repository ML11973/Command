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

#define NBR_OF_MENU 6
#define NBR_OF_FIRST_LEVEL_MENU 3


/************************************************************************/
/* TYPEDEF                                                              */
/************************************************************************/

typedef enum menu_id{
	MAIN,
	MUSIC,
	SETTINGS,
	ALARM,
	MUSIC_PLAYER,
	SETTINGS_HOUR,
	SETTINGS_DATE
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
void _settingsDateMenu(bool firstDraw);

void _mainInput(void);
void _musicInput(void);
void _settingsInput(void);
void _alarmInput(void);
void _musicPlayerInput(void);
void _settingsHourInput(void);
void _settingsDateInput(void);

void _playFile(uint8_t fileNbr);

__attribute__((__interrupt__)) void switch_irq(void);
__attribute__((__interrupt__)) void tc2_irq(void);



/************************************************************************/
/* VARIABLES                                                            */
/************************************************************************/

guiMenu	menus[NBR_OF_MENU + 1]	= {
	_mainMenu, 
	_musicMenu,
	_settingsMenu,
	_alarmMenu, 
	_musicPlayerMenu,
	_settingsHourMenu,
	_settingsDateMenu
};

inputHandler input[NBR_OF_MENU + 1]	= {
	_mainInput, 
	_musicInput, 
	_settingsInput, 
	_alarmInput, 
	_musicPlayerInput,
	_settingsHourInput,
	_settingsDateInput
};

inputBinding binding[NBR_OF_MENU + 1][2] = {{NULL},{_playFile}};	

Color textColor = {GREEN};
	
bool menuChanged = false;
bool needRepaint = false;
uint8_t currentMenuId = 0;

static uint8_t switchState = 0;
static uint8_t selectedCommand = 0;
static bool commandChanged = true;
static uint8_t selectedOption = 1;

static Time editedTime;

static char days[7][9] = {
	"Monday",
	"Tuesday",
	"Wednesday",
	"Thursday",
	"Friday",
	"Saturday",
	"Sunday",
};



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
	
	// Options for waveform generation.
	const tc_waveform_opt_t WAVEFORM_OPT2 = {
		.channel  = TC2_CHANNEL,         // Canal selection.
		.bswtrg   = TC_EVT_EFFECT_NOOP,// Software trigger effect on TIOB.
		.beevt    = TC_EVT_EFFECT_NOOP,// External event effect on TIOB.
		.bcpc     = TC_EVT_EFFECT_NOOP,// RC compare effect on TIOB.
		.bcpb     = TC_EVT_EFFECT_NOOP,// RB compare effect on TIOB.
		.aswtrg   = TC_EVT_EFFECT_NOOP,// Software trigger effect on TIOA.
		.aeevt    = TC_EVT_EFFECT_NOOP,// External event effect on TIOA.
		.acpc     = TC_EVT_EFFECT_NOOP,// RC compare effect on TIOA: toggle.
		.acpa     = TC_EVT_EFFECT_NOOP,// RA compare effect on TIOA
		.wavsel   = TC_WAVEFORM_SEL_UPDOWN_MODE_RC_TRIGGER,// Up mode with automatic trigger
		.enetrg   = false,// External event trigger enable.
		.eevt     = TC_EXT_EVENT_SEL_TIOB_INPUT,// External event selection.
		.eevtedg  = TC_SEL_NO_EDGE,// External event edge selection.
		.cpcdis   = false,     // Counter disable when RC compare.
		.cpcstop  = false,     // Counter clock stopped with RC compare.

		.burst    = TC_BURST_NOT_GATED,     // Burst signal selection.
		.clki     = TC_CLOCK_RISING_EDGE,     // Clock inversion.
		.tcclks   = TC_CLOCK_SOURCE_TC5          // Int. source clock 1 connected to PBA/16
	};
	// 32MHz clock input PBA/2, output checked by scope measure
	
	const tc_interrupt_t TC2_INTERRUPT =
	{
		.etrgs = 0, .ldrbs = 0, .ldras = 0, .cpcs  = 1,
		.cpbs  = 0, .cpas  = 0, .lovrs = 0, .covfs = 0
	};

	// Initialize the timer/counter.
	tc_init_waveform(&AVR32_TC, &WAVEFORM_OPT2);
	tc_write_rc(&AVR32_TC, TC2_CHANNEL, 65000);  // ~7.5Hz
	tc_configure_interrupts(&AVR32_TC, TC2_CHANNEL, &TC2_INTERRUPT);
	INTC_register_interrupt (&tc2_irq, AVR32_TC_IRQ2, AVR32_INTC_INT3);
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
	
	char timeStr[25] = {'T','i','m','e',' ','=',' ',
		currentTime.hours/10 + 48, currentTime.hours % 10 + 48, 'h', ' ',
		currentTime.minutes/10 + 48, currentTime.minutes % 10 + 48, 'm',
		'\0'
	};
	char dateStr[25] = {'D','a','t','e',' ','=',' ',
		currentTime.date/10 + 48, currentTime.date % 10 + 48, '/',
		currentTime.month/10 + 48, currentTime.month % 10 + 48, '/',
		'2', currentTime.year/100 + 48, (currentTime.year/10)%10 + 48, currentTime.year % 10 + 48,
		'\0'
	};
	gfx_BeginNewTerminal((Vector2){20,200});
	gfx_AddOptionToTerminal(days[currentTime.day],8,textColor,false,firstDraw,firstDraw);
	gfx_AddOptionToTerminal(dateStr, 25, textColor,false,firstDraw, firstDraw);
	gfx_AddOptionToTerminal(timeStr , 25, textColor,false,firstDraw, firstDraw);
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
	}
	
	char timeStr[25] = {'T','i','m','e',' ','=',' ',
		currentTime.hours/10 + 48, currentTime.hours % 10 + 48, 'h', ' ',
		currentTime.minutes/10 + 48, currentTime.minutes % 10 + 48, 'm',
		'\0'
	};
	char dateStr[25] = {'D','a','t','e',' ','=',' ',
		currentTime.date/10 + 48, currentTime.date % 10 + 48, '/',
		currentTime.month/10 + 48, currentTime.month % 10 + 48, '/',
		'2', currentTime.year/100 + 48, (currentTime.year/10)%10 + 48, currentTime.year % 10 + 48,
		'\0'
	};
	char colorStr[25] = "textColor = ";
	
	
	gfx_BeginNewTerminal((Vector2){20,200});
	gfx_AddOptionToTerminal(timeStr, 25, textColor, selectedOption & (1<<(0)), firstDraw, firstDraw);
	gfx_AddOptionToTerminal(dateStr, 25, textColor, selectedOption & (1<<(1)), firstDraw, firstDraw);
	gfx_AddOptionToTerminal("textColor = Green", 18, textColor, selectedOption & (1<<(2)), firstDraw, firstDraw);
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
	
	uint8_t h[2] = {editedTime.hours/10 + 48, editedTime.hours % 10 + 48};
	uint8_t m[2] = {editedTime.minutes/10 + 48, editedTime.minutes % 10 + 48};
	
	Color tColor, bgColor;
	if(selectedOption & (1<<0)){
		bgColor = textColor;
		tColor = (Color){BLACK};
	}
	else{
		tColor = textColor;
		bgColor = (Color){BLACK};
	}
	
	screen_SetPixels(Rect(40 + 7*10, 200, 40 + 9*10, 215),bgColor);
	gfx_Label((Vector2){40 + 7 * 10, 200},(char*)h, 2, Small, tColor);
	
	screen_SetPixels(Rect(40 + 9*10, 180, 40 + 11*10, 195),tColor);
	gfx_Label((Vector2){40 + 9 * 10, 180},(char*)m, 2, Small, bgColor);
}

void _settingsDateMenu(bool firstDraw){
	if(firstDraw){
		screen_SetPixels(Rect(0,0,320,240),(Color){BLACK});
		
		_drawSettingsButton();
		
		gfx_BeginNewTerminal((Vector2){20,220});
		gfx_AddLineToTerminal("time -set -hours", 16, textColor, firstDraw);
		
		menuChanged = false;
		
		gfx_BeginNewTerminal((Vector2){40,200});
		gfx_AddLineToTerminal("Year = 2",8,textColor, true);
		gfx_AddLineToTerminal("Month = ",8,textColor, true);
		gfx_AddLineToTerminal("Day = ",6,textColor, true);
	}
	
	uint8_t y[3] = {editedTime.year/100 + 48,(editedTime.year/10)%10 + 48, editedTime.year % 10 + 48};
	uint8_t m[2] = {editedTime.month/10 + 48, editedTime.month % 10 + 48};
	uint8_t d[2] = {editedTime.date/10 + 48, editedTime.date % 10 + 48};
	
	Color tColor, bgColor;
	if(selectedOption & (1<<0)){
		bgColor = textColor;
		tColor = (Color){BLACK};
	}
	else{
		tColor = textColor;
		bgColor = (Color){BLACK};
	}
	
	screen_SetPixels(Rect(40 + 8*10, 200, 40 + (8+3)*10, 215),bgColor);
	gfx_Label((Vector2){40 + 8 * 10, 200},(char*)y, 3, Small, tColor);
	
	if(selectedOption & (1<<1)){
		bgColor = textColor;
		tColor = (Color){BLACK};
	}
	else{
		tColor = textColor;
		bgColor = (Color){BLACK};
	}
	
	screen_SetPixels(Rect(40 + 8*10, 180, 40 + (8+2)*10, 195),bgColor);
	gfx_Label((Vector2){40 + 8 * 10, 180},(char*)m, 2, Small, tColor);
		
	if(selectedOption & (1<<2)){
		bgColor = textColor;
		tColor = (Color){BLACK};
	}
	else{
		tColor = textColor;
		bgColor = (Color){BLACK};
	}
		
	screen_SetPixels(Rect(40 + 6*10, 160, 40 + (9+2)*10, 175),bgColor);
	gfx_Label((Vector2){40 + 6 * 10, 160},(char*)d, 2, Small, tColor);
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
void _mainInput(void) {
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
		selectedOption = 1;
	}
	else if(switchState == 2){
		selectedCommand = (selectedCommand == NBR_OF_FIRST_LEVEL_MENU - 1)?(0):(selectedCommand+1);
		commandChanged = true;
		screen_SetPixels(Rect(0,40,320,50),(Color){BLACK});
	}
	else if(switchState == 3){
		selectedCommand = (selectedCommand == 0)?(NBR_OF_FIRST_LEVEL_MENU - 1):(selectedCommand-1);
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
	else if(switchState == 2){
		(selectedOption &(1<<0))?(selectedOption = (1<<(3-1))):(selectedOption>>=1);
	}
	else if (switchState == 3){
		(selectedOption &(1<<(3-1)))?(selectedOption = (1<<0)):(selectedOption<<=1);
	}
	else if(switchState == 4){
		switch (selectedOption)
		{
		case 1:
			editedTime = currentTime;
			currentMenuId = SETTINGS_HOUR;
			selectedOption = 1;
			break;
		case 2:
			editedTime = currentTime;
			currentMenuId = SETTINGS_DATE;
			selectedOption = 1;
			break;
		case 4:
			textColor.value = (textColor.value == GREEN)?( WHITE):(GREEN);
			break;
		}
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
				editedTime.hours++;
				if(editedTime.hours > 24)
					editedTime.hours = 0;
				break;
			case 2:
				editedTime.minutes++;
				if(editedTime.minutes > 60)
					editedTime.minutes = 0;
				break;
			/*case 2:
				currentTime.seconds++;
				break;*/
		}
	}
	else if(switchState == 3){
		switch(selectedOption){
			case 1:
				editedTime.hours--;
				if(editedTime.hours > 24)
				editedTime.hours = 24;
				break;
			case 2:
				editedTime.minutes--;
				if(editedTime.minutes > 60)
					editedTime.minutes = 60;
				break;
			/*case 2:
				currentTime.seconds--;
				break;*/
		}
	}
	else if(switchState == 4){
		selectedOption<<=1;
		
		//Hour, minutes /*and seconds*/ are set
		if(selectedOption > 2){
			currentMenuId = SETTINGS;
			menuChanged = true;
			
			currentTime.hours = editedTime.hours;
			currentTime.minutes = editedTime.minutes;
			
			rtc_setTime();
			rtc_usart_sendTimeToDisplay();
			selectedOption = 1;
		}
	}
	switchState = 0;
}

void _settingsDateInput(void){
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
				editedTime.year++;
				if(editedTime.year > 199)
					editedTime.year = 0;
				break;
			case 2:
				editedTime.month++;
				if(editedTime.month > 12)
					editedTime.month = 0;
				break;
			case 4:
				editedTime.date++;
				if(editedTime.date > 31)
					editedTime.date = 0;
				break;
		}
	}
	else if(switchState == 3){
		switch(selectedOption){
			case 1:
				editedTime.year--;
				if(editedTime.year > 199)
					editedTime.year = 199;
				break;
			case 2:
				editedTime.month--;
				if(editedTime.month > 12)
					editedTime.month = 12;
				break;
			case 4:
				editedTime.date--;
				if(editedTime.date > 31)
					editedTime.date = 31;
				break;
		}
	}
	else if(switchState == 4){
		selectedOption<<=1;
		
		//Year, month and day are set
		if(selectedOption > 4){
			currentMenuId = SETTINGS;
			menuChanged = true;
			
			currentTime.year = editedTime.year;
			currentTime.month = editedTime.month;
			currentTime.date = editedTime.date;
			
			rtc_setTime();
			selectedOption = 1;
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

/************************************************************************/
/* INTERRUPT                                                            */
/************************************************************************/

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
	
	tc_start(&AVR32_TC, TC2_CHANNEL);
	gpio_disable_pin_interrupt(PIN_SWITCH0);
	gpio_disable_pin_interrupt(PIN_SWITCH1);
	gpio_disable_pin_interrupt(PIN_SWITCH2);
	gpio_disable_pin_interrupt(PIN_SWITCH3);
}


__attribute__((__interrupt__)) void tc2_irq(void){
	tc_stop(&AVR32_TC, TC2_CHANNEL);
	gpio_enable_pin_interrupt(PIN_SWITCH0, GPIO_RISING_EDGE);
	gpio_enable_pin_interrupt(PIN_SWITCH1, GPIO_RISING_EDGE);
	gpio_enable_pin_interrupt(PIN_SWITCH2, GPIO_RISING_EDGE);
	gpio_enable_pin_interrupt(PIN_SWITCH3, GPIO_RISING_EDGE);
	
	AVR32_TC.channel[TC2_CHANNEL].SR;
}
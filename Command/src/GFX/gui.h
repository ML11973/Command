/*
 * gui.h
 *
 * Created: 10.11.2017 11:19:57
 *  Author: QVT
 */ 


#ifndef GUI_H_
#define GUI_H_

/************************************************************************/
/* INCLUDES                                                             */
/************************************************************************/

#include <asf.h>


/************************************************************************/
/* DEFINITIONS                                                          */
/************************************************************************/

typedef void (*guiMenu[])(bool);
typedef void (*inputHandler[])();
typedef void (*inputBinding[])(int);


/************************************************************************/
/* VARIABLES                                                            */
/************************************************************************/

extern guiMenu menus;
extern inputHandler input;

extern bool menuChanged;
extern bool needRepaint;
extern uint8_t currentMenuId;

/************************************************************************/
/* FUNCTIONS	                                                        */
/************************************************************************/

void gui_Init(void);

/* gui_loadingScreen
 *
 * Draw the loading screen
 *
 * Created 10.11.17 QVT
 * Last modified 10.11.17 QVT
 */
void gui_loadingScreen(void);


#endif /* GUI_H_ */
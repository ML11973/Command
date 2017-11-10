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

typedef void (*guiMenuFunc)(bool);
typedef void (*inputFunc)(void);
typedef struct{
	guiMenuFunc draw;
	inputFunc input;
}Menu;


/************************************************************************/
/* VARIABLES                                                            */
/************************************************************************/

extern Menu *menus;

/************************************************************************/
/* FUNCTIONS	                                                        */
/************************************************************************/

/* gui_InitMenus
 *
 * Called to initialise "menus" using malloc
 *
 * Created 10.11.17 QVT
 * Last modified 10.11.17 QVT
 */
bool gui_InitMenus();

/* gui_loadingScreen
 *
 * Draw the main menu
 *
 * Created 10.11.17 QVT
 * Last modified 10.11.17 QVT
 */
void gui_loadingScreen(void);


#endif /* GUI_H_ */
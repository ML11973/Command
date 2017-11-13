/*
 * GFX.h
 *
 * Created: 06.11.2017 14:14:01
 *  Author: voletqu
 */ 


#ifndef GFX_H_
#define GFX_H_

#include <asf.h>

#include "vector2.h"
#include "rectangle.h"
#include "color.h"
#include "screen.h"

typedef enum{
	Small,
	Medium,
	Large
}TextSize;


void gfx_DrawLine(Vector2 start, Vector2 end, Color color, uint8_t width);

void gfx_DrawPixel(Vector2 position, Color color);

void gfx_DrawCircle(Vector2 center, uint16_t radius, uint8_t width, Color color);

void gfx_DrawRectangle(Rectangle rect, Color color, bool isFilled);

void gfx_DrawTriangle(Vector2 firstCorner, Vector2 secondCorner, Vector2 ThirdCorner, Color color, bool isFilled);

void gfx_FillScreen(Color color);

void gfx_Label(Vector2 position, char *content, uint8_t contentSize, TextSize textSize, Color color);

void gfx_BeginNewTerminal(Vector2 topLeft);

void gfx_AddLineToTerminal(char *content, uint8_t contentSize, Color color, bool isDelayed);

void gfx_AddOptionToTerminal(char *content, uint8_t contentSize, Color color, bool isSelected, bool firstDraw, bool isDelayed);

void gfx_DrawTerminalButton(Vector2 position, char *shortcut, char *description, uint8_t size, Color color);

void gfx_StartNonStaticArea(void);

void gfx_EndNonStaticArea(void);

void gfx_cmdLine(char *content, uint8_t contentSize, Color color, bool fisrtDraw);

#endif /* GFX_H_ */
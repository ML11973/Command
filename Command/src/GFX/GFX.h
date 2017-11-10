/*
 * GFX.h
 *
 * Created: 06.11.2017 14:14:01
 *  Author: voletqu
 */ 


#ifndef GFX_H_
#define GFX_H_

#include <asf.h>

#include "Vector2.h"
#include "Rectangle.h"
#include "Color.h"
#include "Screen.h"

typedef void (*buttonCallback) (void);

typedef enum{
	Small,
	Medium,
	Large
}TextSize;


void GFX_DrawLine(Vector2 start, Vector2 end, Color color, uint8_t width);

void GFX_DrawPixel(Vector2 position, Color color);

void GFX_DrawCircle(Vector2 center, uint16_t radius, uint8_t width, Color color);

void GFX_DrawRectangle(Rectangle rect, Color color, bool isFilled);

void GFX_DrawTriangle(Vector2 firstCorner, Vector2 secondCorner, Vector2 ThirdCorner, Color color, bool isFilled);

void GFX_FillScreen(Color color);

void GFX_Label(Vector2 position, volatile char *content, uint8_t contentSize, TextSize textSize, Color color);

void GFX_SelectedLabel(Rectangle rect, char *content, Color textColor, Color backgroundColor);

void GFX_Button(Rectangle rect, char *content, buttonCallback callback);

#endif /* GFX_H_ */
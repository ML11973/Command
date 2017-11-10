/*
 * Rectangle.h
 *
 * Created: 08.11.2017 15:52:32
 *  Author: voletqu
 */ 


#ifndef RECTANGLE_H_
#define RECTANGLE_H_

#include <asf.h>
#include "vector2.h"

struct rectangle
{
	Vector2 bottomLeft;
	Vector2 topRight;
};
typedef struct rectangle Rectangle;

uint16_t rectangle_GetWidth(Rectangle rect);
uint8_t  rectangle_GetHeight(Rectangle rect);

uint32_t rectangle_GetArea(Rectangle rect);

void rectangle_VerifySize(Rectangle *rect, uint16_t maxWidth, uint8_t maxHeight);

Rectangle Rect(uint16_t x1, uint8_t y1, uint16_t x2, uint8_t y2);

#endif /* RECTANGLE_H_ */
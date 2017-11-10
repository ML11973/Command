/*
 * Rectangle.c
 *
 * Created: 08.11.2017 15:52:18
 *  Author: voletqu
 */ 
#include "rectangle.h"

uint16_t rectangle_GetWidth(Rectangle rect){
	return rect.topRight.x - rect.bottomLeft.x;
}

uint8_t  rectangle_GetHeight(Rectangle rect){
	return rect.topRight.y - rect.bottomLeft.y;
}

uint32_t rectangle_GetArea(Rectangle rect){
	volatile uint32_t value = (uint32_t)(rect.topRight.x - rect.bottomLeft.x +1)*(uint32_t)(rect.topRight.y - rect.bottomLeft.y +1);
	return value;
}

void rectangle_VerifySize(Rectangle *rect, uint16_t maxWidth, uint8_t maxHeight){
	if(rect->bottomLeft.x > maxWidth)
		rect->bottomLeft.x = maxWidth;

	if(rect->topRight.x > maxWidth)
		rect->topRight.x = maxWidth;

	if(rect->bottomLeft.y > maxHeight)
		rect->bottomLeft.y = maxHeight;

	if(rect->topRight.y > maxHeight)
		rect->topRight.y = maxHeight;

	//if(rect->topRight.x == rect->bottomLeft.x)
		//rect->topRight.x++;
	//if(rect->topRight.y == rect->bottomLeft.y)
		//rect->topRight.y++;
}


Rectangle Rect(uint16_t x1, uint8_t y1, uint16_t x2, uint8_t y2){
	//Vector2 tl = {x1,y1};
	//Vector2 br = {x2, y2};
	//Rectangle r = {tl, br};
	return (Rectangle){{x1,y1},{x2,y2}};
}
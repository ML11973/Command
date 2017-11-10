/*
 * Screen.h
 *
 * Created: 06.11.2017 14:13:38
 *  Author: voletqu
 */ 


#ifndef SCREEN_H_
#define SCREEN_H_

#include "Vector2.h"
#include "Rectangle.h"
#include "Color.h"

void Screen_Init();

void Screen_SetBacklightStrength(uint8_t strength);

void Screen_TouchZone(Rectangle rect, void *callback(void));

void Screen_SetPixel(Vector2 position, Color color);

void Screen_SetPixels(Rectangle rect, Color color);

#endif /* SCREEN_H_ */
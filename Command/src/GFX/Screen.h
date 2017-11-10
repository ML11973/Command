/*
 * Screen.h
 *
 * Created: 06.11.2017 14:13:38
 *  Author: voletqu
 */ 


#ifndef SCREEN_H_
#define SCREEN_H_

#include "vector2.h"
#include "rectangle.h"
#include "color.h"

void screen_Init();

void screen_SetBacklightStrength(uint8_t strength);

void screen_SetPixel(Vector2 position, Color color);

void screen_SetPixels(Rectangle rect, Color color);

#endif /* SCREEN_H_ */
/*
 * Vector2.h
 *
 * Created: 08.11.2017 15:53:44
 *  Author: voletqu
 */ 


#ifndef VECTOR2_H_
#define VECTOR2_H_

#include <asf.h>

struct vector2
{
	uint16_t x;	//320 => 16 bits
	uint8_t y;	//240 => 8 bits
};
typedef struct vector2 Vector2;

Vector2 Vector2_Add(Vector2 v1, Vector2 v2);

#endif /* VECTOR2_H_ */
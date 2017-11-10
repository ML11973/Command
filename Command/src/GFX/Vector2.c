/*
 * Vector2.c
 *
 * Created: 08.11.2017 15:53:31
 *  Author: voletqu
 */ 

#include "Vector2.h"

Vector2 Vector2_Add(Vector2 v1, Vector2 v2){
	return (Vector2){.x = v1.x + v2.x,
					 .y = v1.y + v2.y};
}
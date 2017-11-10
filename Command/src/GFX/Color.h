/*
 * Color.h
 *
 * Created: 08.11.2017 15:56:45
 *  Author: voletqu
 */ 


#ifndef COLOR_H_
#define COLOR_H_

#include <asf.h>

#define		BLACK		0x0000		//  B  G  R
#define		GREEN		0x07E0		// 0 0 0 0  0 1 1 1  1 1 1 0  0 0 0 0
#define		RED			0x001F		// 0 0 0 0  0 0 0 0  0 0 0 1  1 1 1 1
#define		BLUE		0xF800		// 1 1 1 1  1 0 0 0  0 0 0 0  0 0 0 0
#define		WHITE		0xFFFF
#define		YELLOW		0x07FF
#define		MAGENTA		0xFCEF		// 1 1 1 1  1 1 0 0  1 1 1 0  1 1 1 1
#define		CYAN		0xFFF0		// 1 1 1 1  1 1 1 1  1 1 1 1  0 0 0 0

#define		GREY		0x4208		// 0 1 0 0  0 0 1 0  0 0 0 0  1 0 0 0
#define     LIGHT_GREY  0x8410		// 1 0 0 0  0 1 0 0  0 0 0 1  0 0 0 0
#define		LIGHT_RED	0x0013		// 0 0 0 0  0 0 0 0  0 0 0 1  0 0 1 1
#define		LIGHT_GREEN	0x0260		// 0 0 0 0  0 0 1 0  0 1 1 0  0 0 0 0
#define		LIGHT_BLUE	0xFC10		// 1 1 1 1  1 1 0 0  0 0 0 1  0 0 0 0

union color{
	uint16_t value;
	struct
	{
		uint8_t b : 5;
		uint8_t g : 6;
		uint8_t r : 5;
	} components;
};
typedef union color Color;

#endif /* COLOR_H_ */
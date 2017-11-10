/*
 * GFX.c
 *
 * Created: 06.11.2017 14:14:06
 *  Author: voletqu
 */ 

#include "gfx.h"
#include "fonts.h"

void gfx_DrawLine(Vector2 start, Vector2 end, Color color, uint8_t width){
	/*uint16_t progress = 0;
	uint16_t deltaX = abs(start.x - end.x);
	uint16_t deltaY = abs(start.y - end.y);
	uint16_t left = width * 0.5f;
	uint16_t right = (width - 1) * 0.5f;

	if(deltaX == 0){
		Screen_SetPixels(Rect(start.x - left, start.y, end.x + right, end.y),color);
	}
	else if(deltaY == 0){
		Screen_SetPixels(Rect(start.x, start.y - left, end.x, end.y + right),color);
	}
	else if(deltaX > deltaY) {
		float slope = deltaX / (float)(deltaY);
		uint16_t segmentLength = (uint16_t)slope;

		uint16_t longerSegmentFrequency = 1/(slope-segmentLength);

		for(uint8_t i = 0; i < deltaX; i++) {
			uint16_t distance = (i%longerSegmentFrequency == 0)?(segmentLength):(segmentLength + 1);
			Screen_SetPixels(Rect(start.x + progress - left, start.y + i, distance + right, start.y + i),color);
			progress += distance;
		}
	}
	else {
		volatile float slope = (deltaY) /(float) (deltaX);
		uint16_t segmentLength = (uint16_t)slope;
		volatile uint16_t longerSegmentFrequency = 1/(slope-segmentLength);

		for(uint8_t i = 0; i < deltaY; i++) {
			uint16_t distance = (i%longerSegmentFrequency == 0)?(segmentLength):(segmentLength + 1);
			Screen_SetPixels(Rect(start.x + i, start.y + progress - left, start.x + i, start.y + distance + right),color);
			progress += distance;
		}
	}*/
	// Compute deltas, ie. "width" and "height" of line, then
	// compute x and y direction, and make deltas positive for later use.
	S16 xinc = 1; // Start off assuming direction is positive, ie. right.
	S16 dx = end.x - start.x;
	if (dx < 0){xinc = -1; dx = -dx; }

	S16 yinc = 1; // Start off assuming direction is positive, ie. down.
	S16 dy = end.y - start.y;
	if (dy < 0){yinc = -1; dy = -dy; }
	// Set up current point.
	uint16_t x = start.x;
	uint16_t y = start.y;
	uint16_t i, j, k;

	// A "flat" line (dx>dy) is handled differently from a "steep" line (dx<dy).
	if (dx > dy) {
		// Walk along X, draw pixel, and step Y when required.
		S16 e = dx >> 1;
		for ( i = 0; i <= dx; ++i) {
			for (j=width ; j>0 ; j--)
			{
				k = (U16) (j / 2);
				if ((j & 0x0001)) { screen_SetPixel((Vector2){x,y+k},color); }
				else { screen_SetPixel((Vector2){x,y-k},color); }
			}
			// Sub-pixel "error" overflowed, so we step Y and reset the "error".
			if (e <= 0){
				e += dx;
				y += yinc;
			}
			// Walk one step along X.
			e -= dy;
			x += xinc;
		}
		} else {
		// Walk along Y, draw pixel, and step X when required.
		S16 e = dy >> 1;
		for (i = 0; i <= dy; ++i) {
			for (j=width ; j>0 ; j--)
			{
				k = (U16) (j / 2);
				if ((j & 0x0001)){ screen_SetPixel((Vector2){x+k,y},color); }
				else { screen_SetPixel((Vector2){x-k,y},color); }
			}
			// Sub-pixel "error" overflowed, so we step X and reset the "error".
			if (e <= 0){
				e += dy;
				x += xinc;
			}
			// Walk one step along Y.
			e -= dx;
			y += yinc;
		}
	}
}

void gfx_DrawPixel(Vector2 position, Color color){
	screen_SetPixel(position,color);
}

void gfx_DrawCircle(Vector2 center, uint16_t radius, uint8_t width, Color color){
	int16_t x, y, e;
	x = 0;
	y = radius;
	e = 1 - radius;
	
	do
	{
		if(width == radius)
		{
			screen_SetPixels(Rect(center.x -x, center.y +y, center.x+x, center.y +y),color);
			screen_SetPixels(Rect(center.x -x, center.y -y, center.x+x, center.y -y),color);
			screen_SetPixels(Rect(center.x -y, center.y +x, center.x+y, center.y +x),color);
			screen_SetPixels(Rect(center.x -y, center.y -x, center.x+y, center.y -x),color);
		}
		else
		{
			/*Screen_SetPixel(Vector2_Add((Vector2){ x, y },center),color);
			Screen_SetPixel(Vector2_Add((Vector2){-x, y },center),color);
			Screen_SetPixel(Vector2_Add((Vector2){ x, -y},center),color);
			Screen_SetPixel(Vector2_Add((Vector2){-x, -y},center),color);
			Screen_SetPixel(Vector2_Add((Vector2){ y, x },center),color);
			Screen_SetPixel(Vector2_Add((Vector2){-y, x },center),color);
			Screen_SetPixel(Vector2_Add((Vector2){ y, -x},center),color);
			Screen_SetPixel(Vector2_Add((Vector2){-y, -x},center),color);*/
				
			screen_SetPixels(Rect(center.x + x, center.y + y - width, center.x + x, center.y + y),color);
			screen_SetPixels(Rect(center.x + x, center.y - y, center.x + x, center.y - y + width),color);
			screen_SetPixels(Rect(center.x - x, center.y + y - width, center.x - x, center.y + y),color);
			screen_SetPixels(Rect(center.x - x, center.y - y, center.x - x, center.y - y + width),color);
			
			screen_SetPixels(Rect(center.x + y - width, center.y + x, center.x + y, center.y + x),color);
			screen_SetPixels(Rect(center.x + y - width, center.y - x, center.x + y, center.y - x),color);
			screen_SetPixels(Rect(center.x - y, center.y + x, center.x - y + width, center.y + x),color);
			screen_SetPixels(Rect(center.x - y, center.y - x, center.x - y + width, center.y - x),color);
		}

		if(e < 0)
			e += 3 + 2 * x;
		else
			e += 5 + 2 * (x - y--);
		x++;
	} while(x  <= y);
}

void gfx_DrawRectangle(Rectangle rect, Color color, bool isFilled){
	
	
}

void gfx_DrawTriangle(Vector2 firstCorner, Vector2 secondCorner, Vector2 ThirdCorner, Color color, bool isFilled){


}

void gfx_FillScreen(Color color){


}

void gfx_Label(Vector2 position, char *content, uint8_t contentSize,TextSize textSize, Color color){
	if(*content == '\0' || contentSize == 0)
		return;
	Vector2 fontSize = {8,8};
	if(textSize == Medium){
		fontSize.x = 20;
		fontSize.y = 16;
	}
		
	for(uint8_t n = 0; n < contentSize; n++, content++){
		if(*content == '\0')
			break;
		
		for(uint8_t x = 0; x < fontSize.x; x++){
			uint16_t mask = 1;
			for(uint8_t y = 0; y < fontSize.y; y++){
				if(textSize == Medium){
					if(FONT16x12[*content - 31][x] & mask)
						screen_SetPixel(vector2_Add(position,(Vector2){n * 14 + 16 - y, 12 - x}), color);
				}else{
					if(FONT8x8[*content - 31][x] & (1<<y))
						screen_SetPixel(vector2_Add(position,(Vector2){n * 10 + 8 - y, 8 - x}), color);
				}
				mask <<= 1;
			}
		}
	}
}
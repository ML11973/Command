/*
 * rtc.h
 *
 * Created 13.11.17 MLN
 * Last modified 13.11.17
 */ 


#ifndef RTC_H
#define RTC_H

/************************************************************************/
/* INCLUDES                                                             */
/************************************************************************/

#include <asf.h>
#include "GFX/gfx.h"



/************************************************************************/
/* DEFINITIONS                                                          */
/************************************************************************/

#define MAXALARMNUMBER 10
#define REFERENCE_YEAR 2000

typedef struct time {
	uint8_t year		:8;		// 0-255 -> 2000-2255
	uint8_t month		:8;		// 1-12
	uint8_t date		:6;		// 1-31
	uint8_t day			:3;		// 1-7
	uint8_t hours		:7;		// 0-23
	uint8_t minutes		:7;		// 0-59
	uint8_t seconds		:7;		// 0-59
} Time;

typedef struct alarm {
	Time alarmTime;
	bool alarmEnable;
	uint8_t ringtoneIndex;
} Alarm;



/************************************************************************/
/* VARIABLES                                                            */
/************************************************************************/

extern Time currentTime;
extern Alarm alarm[MAXALARMNUMBER];



/************************************************************************/
/* FUNCTIONS	                                                        */
/************************************************************************/

/* rtc_readData
 *
 * Description
 *
 * Created 15.11.17 MLN
 * Last modified 15.11.17 MLN
 */
void rtc_read(uint8_t, uint8_t);



/* rtc_write
 *
 * Description
 *
 * Created 16.11.17 MLN
 * Last modified 16.11.17 MLN
 */
void rtc_write(uint8_t, uint8_t);



/* rtc_setAddress
 *
 * Description
 *
 * Created 15.11.17 MLN
 * Last modified 15.11.17 MLN
 */
void rtc_setAddress(uint8_t);



/* fonction
 *
 * Description
 *
 * Created 10.11.17 MLN
 * Last modified 10.11.17 MLN
 */
void rtc_setTime(void);

/* fonction
 *
 * Description
 *
 * Created 10.11.17 MLN
 * Last modified 10.11.17 MLN
 */
void rtc_getTime(void);

/* rtc_getTime
 *
 * Description
 *
 * Created 15.11.17 MLN
 * Last modified 15.11.17 MLN
 */
void rtc_setNextMinuteInterrupt(void);

/* rtc_getTime
 *
 * Description
 *
 * Created 15.11.17 MLN
 * Last modified 15.11.17 MLN
 */
void rtc_setNextAlarm(Alarm alarm[]);


#endif /* RTC_H */
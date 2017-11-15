/*
 * rtc.c
 *
 * Created 13.11.17 MLN
 * Last modified 13.11.17 MLN
 * 
 */ 

/************************************************************************/
/* INCLUDES                                                             */
/************************************************************************/

#include "rtc.h"



/************************************************************************/
/* DEFINITIONS                                                          */
/************************************************************************/

// Parameters already defined in init.c, redefined as a precaution
#ifndef RTC_ADDRESS_WRITE
#define TWI_MASTER_SPEED 100000
#define RTC_ADDRESS_WRITE 0xD0
#endif

#define		RTC_TWI						&AVR32_TWI			//! TWIM Module Used
#define		RTC_ADDRESS_READ 			0xD1		        //! Target's TWI address
#define		RTC_ADDR_LGT				0					//! Internal Address length
#define		VIRTUALMEM_ADDR1			0x00				//! Internal Address
#define		VIRTUALMEM_ADDR2			0x00				//! Internal Address
#define		VIRTUALMEM_ADDR3			0x00				//! Internal Address

#define		BUFFER_SIZE					10

// Timekeeping registers addresses
#define		RTC_SECONDS				0x00
#define		RTC_MINUTES				0x01
#define		RTC_HOURS				0x02
#define		RTC_DAY					0x03
#define		RTC_DATE				0x04
#define		RTC_MONTH				0x05
#define		RTC_YEAR				0x06
#define		RTC_ALARM1_SECONDS		0x07
#define		RTC_ALARM1_MINUTES		0x08
#define		RTC_ALARM1_HOURS		0x09
#define		RTC_ALARM1_DAYDATE		0x0A
#define		RTC_ALARM2_MINUTES		0x0B
#define		RTC_ALARM2_HOURS		0x0C
#define		RTC_ALARM2_DAYDATE		0x0D
#define		RTC_TEMP_MSB			0x11
#define		RTC_TEMP_LSB			0x12

#define		RTC_CONTROL				0x0E
#define		RTC_STATUS				0x0F


// Registre status 1 du RTC
#define		EOSC						7					// enable oscillator
#define		BBSQW						6					// batterie backed aquare-wave enable
#define		CONV						5					// Convert temperature
#define		INTCN						2					// Interrupt control
#define		A2IE						1					// Alarm 1 interrupt enable
#define		A1IE						0					// Alarm 2 interrupt enable

// Registre status 2 du RTC
#define		OSF							7					// Oscillator stop flag
#define		BB32KHZ						6					// Battery Backed 32KHz output
#define		EN32KHZ						3					// Enable 32KHz output
#define		BSY							2					// Busy
#define		A2F							1					// Alarm 2 flag
#define		A1F							0					// Alarm 1 flag

// Le 7ème bit AxMx
#define		BitE						7					// Enable Alarm



/************************************************************************/
/* VARIABLES                                                            */
/************************************************************************/

Time currentTime;
Alarm alarm[MAXALARMNUMBER];
	
uint8_t buffer [] = {0, 0, 0};

twi_package_t packet = {
	.chip = RTC_ADDRESS_READ,
	.addr[0] = VIRTUALMEM_ADDR1,
	.addr[1] = VIRTUALMEM_ADDR2,
	.addr[2] = VIRTUALMEM_ADDR3,
	.addr_length = RTC_ADDR_LGT,
	.buffer = &buffer,
	.length = sizeof(buffer)/sizeof(uint8_t),
};



/************************************************************************/
/* FUNCTIONS                                                            */
/************************************************************************/

/* rtc_setAddress
 *
 * Description
 *
 * Created 15.11.17 MLN
 * Last modified 15.11.17 MLN
 */
void rtc_setAddress(uint8_t address){
	
	
	
}



/* rtc_setTime
 *
 * Description
 *
 * Created 15.11.17 MLN
 * Last modified 15.11.17 MLN
 */
void rtc_setTime(Time time){
	twi_master_enable(RTC_TWI);
	
	
	
	twi_master_disable(RTC_TWI);
}



/* rtc_getTime
 *
 * Description
 *
 * Created 15.11.17 MLN
 * Last modified 15.11.17 MLN
 */
void rtc_getTime(void){
	
	char stringOutput[6];
	
	twi_master_enable(RTC_TWI);
	/*// Clearing buffer
	buffer[0] = 0;
	buffer[1] = 0;
	buffer[2] = 0;*/
	// Fetching data from Seconds, Minutes and Hours RTC registers 
	packet.addr[0] = RTC_SECONDS;
	packet.addr[1] = RTC_MINUTES;
	packet.addr[2] = RTC_HOURS;
	twi_master_read(RTC_TWI, &packet);
	
	// Copying fetched data to current time value
	currentTime.seconds = packet.addr[0];
	currentTime.minutes = packet.addr[1];
	currentTime.hours	= packet.addr[2];
	
	// Fetching data from Day, Date and Month RTC registers
	packet.addr[0] = RTC_DAY;
	packet.addr[1] = RTC_DATE;
	packet.addr[2] = RTC_MONTH;
	twi_master_read(RTC_TWI, &packet);
	
	// Copying fetched data to current time value
	currentTime.day		= packet.addr[0];
	currentTime.date	= packet.addr[1];
	currentTime.month	= packet.addr[2];
	
	// Fetching data from Year RTC register
	packet.addr[0] = RTC_YEAR;
	packet.addr[1] = 0;
	packet.addr[2] = 0;
	twi_master_read(RTC_TWI, &packet);
	
	// Copying fetched data to current time value
	currentTime.year	= packet.addr[0];
	
	twi_master_disable(RTC_TWI);
	
	//Converting fetched data to decimal
	currentTime.seconds = 10 * (currentTime.seconds >> 4) + (currentTime.seconds & 0x0F);
	
	currentTime.minutes = 10 * (currentTime.minutes >> 4) + (currentTime.minutes & 0x0F);
	
	currentTime.hours	= 10 * ((currentTime.hours & 0x30) >> 4) + (currentTime.hours & 0x0F);
	// We don't need to convert the day number
	currentTime.date	= 10 * (currentTime.date >> 4) + (currentTime.date & 0x0F);
	// Since a bit in the month register indicates the century, we compute years before months
	currentTime.year	= 100 * ((currentTime.month & 0x80) >> 7) * (currentTime.year >> 4) + (currentTime.year & 0x0F);
	
	currentTime.month	= 10 * ((currentTime.month & 0x10) >> 4) + (currentTime.month & 0x0F);
	
	stringOutput[0] = currentTime.hours / 10 + 48;
	stringOutput[1] = currentTime.hours % 10 + 48;
	stringOutput[2] = currentTime.minutes / 10 + 48;
	stringOutput[3] = currentTime.minutes % 10 + 48;
	stringOutput[4] = currentTime.date / 10 + 48;
	stringOutput[5] = currentTime.date % 10 + 48;
	
	gfx_AddLineToTerminal(stringOutput, 6, (Color){WHITE}, 0);
}



/* rtc_setNextMinuteInterrupt
 *
 * Description
 *
 * Created 15.11.17 MLN
 * Last modified 15.11.17 MLN
 */
void rtc_setNextMinuteInterrupt(void){
	
	
}



/* rtc_setNextAlarm
 *
 * Sets the next alarm chronologically in the Alarm2 register of the RTC
 *
 * Created 15.11.17 MLN
 * Last modified 15.11.17 MLN
 */
void rtc_setNextAlarm(Alarm alarm[]){
	
	
}
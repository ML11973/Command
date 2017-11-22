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
#define RTC_ADDRESS_WRITE 0xD0>>1
#endif

#define		RTC_TWI						&AVR32_TWI			//! TWIM Module Used
#define		RTC_ADDRESS_READ 			0xD1>>1		        //! Target's TWI address
#define		RTC_ADDR_LGT				0					//! Internal Address length
#define		VIRTUALMEM_ADDR1			0x00				//! Internal Address
#define		VIRTUALMEM_ADDR2			0x00				//! Internal Address
#define		VIRTUALMEM_ADDR3			0x00				//! Internal Address

#define		BUFFER_SIZE					14

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
	
uint8_t sentData [BUFFER_SIZE] = {0};
uint8_t buffer [BUFFER_SIZE] = {0};







/************************************************************************/
/* FUNCTIONS                                                            */
/************************************************************************/


/* rtc_read
 *
 * Reads registers from firstRegister to firstRegister + dataNumber.
 * Stores data in buffer file-restricted global array.
 *
 * Created 15.11.17 MLN
 * Last modified 16.11.17 MLN
 */
void rtc_read(uint8_t firstRegister, uint8_t dataNumber){
	
	// Clearing buffer table
	for (uint8_t i = 0; i < dataNumber; i++){
		buffer[i] = 0;
	}
	
	// Packet for read ASF library function
	twi_package_t readPacket = {
		.chip = RTC_ADDRESS_READ,
		.addr[0] = firstRegister,	// DS2323 RTC automatically increments registers
		.addr[1] = 0,
		.addr[2] = 0,
		.addr_length = 1,
		.buffer = &buffer,			// Data is stored in this table
		.length = dataNumber,		// Number of bytes to store
	};
	
	twi_master_read(RTC_TWI, &readPacket);
}



/* rtc_write
 *
 * Writes registers from firstRegister to firstRegister + dataNumber.
 * Uses data from sentData file-restricted global array.
 *
 * Created 16.11.17 MLN
 * Last modified 16.11.17 MLN
 */
void rtc_write(uint8_t firstRegister, uint8_t dataNumber){
	
	// Packet for write ASF library function
	twi_package_t writePacket = {
		.chip = RTC_ADDRESS_WRITE,
		.addr[0] = firstRegister,	// DS2323 RTC automatically increments registers
		.addr[1] = 0,
		.addr[2] = 0,
		.addr_length = 1,
		.buffer = &sentData,		// Modify sentData before calling function
		.length = dataNumber,		// Number of bytes to send
	};
	
	twi_master_write(RTC_TWI, &writePacket);
}



/* rtc_setTime
 *
 * Converts currentTime to BCD and sends it to RTC.
 *
 * Created 15.11.17 MLN
 * Last modified 15.11.17 MLN
 */
void rtc_setTime(void){
	// Time to BCD conversion
	
	
	twi_master_enable(RTC_TWI);
	
}



/* rtc_getTime
 *
 * TO BE MODIFIED, test function
 *
 * Created 15.11.17 MLN
 * Last modified 22.11.17 MLN
 */
void rtc_getTime(void){
	volatile uint8_t status = 0;
	char stringOutput[7];
	
	twi_master_enable(RTC_TWI);
	sentData[0] = 0x10;
	sentData[1] = 0x17;
	sentData[2] = 0x19;
	sentData[3] = 0x03;
	rtc_write(RTC_SECONDS, 4);
	rtc_read(RTC_SECONDS, 4);
	
	currentTime.seconds = buffer[0];
	currentTime.minutes = buffer[1];
	currentTime.hours	= buffer[2];
	currentTime.day		= buffer[3];
	
	// KEEP THIS PART
	// Converting fetched data to decimal
	currentTime.seconds = 10 * (currentTime.seconds >> 4) + (currentTime.seconds & 0x0F);
	
	currentTime.minutes = 10 * (currentTime.minutes >> 4) + (currentTime.minutes & 0x0F);
	
	currentTime.hours	= 10 * ((currentTime.hours & 0x30) >> 4) + (currentTime.hours & 0x0F);
	// We don't need to convert the day number
	currentTime.date	= 10 * (currentTime.date >> 4) + (currentTime.date & 0x0F);
	// Since a bit in the month register indicates the century, we compute years before months
	currentTime.year	= 100 * ((currentTime.month & 0x80) >> 7) * (currentTime.year >> 4) + (currentTime.year & 0x0F) + REFERENCE_YEAR;
	
	currentTime.month	= 10 * ((currentTime.month & 0x10) >> 4) + (currentTime.month & 0x0F);
	// END KEEP
	
	
	stringOutput[0] = currentTime.hours / 10 + 48;
	stringOutput[1] = currentTime.hours % 10 + 48;
	stringOutput[2] = currentTime.minutes / 10 + 48;
	stringOutput[3] = currentTime.minutes % 10 + 48;
	stringOutput[4] = currentTime.seconds / 10 + 48;
	stringOutput[5] = currentTime.seconds % 10 + 48;
	stringOutput[6] = currentTime.day + 48;
	
	gfx_AddLineToTerminal(stringOutput, 7, (Color){WHITE}, 0);
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



/* _timeToBCD
 *
 * Converts a Time variable to BCD (format used by RTC).
 * Returns table acceptable by RTC.
 * NOT CHECKED YET
 *
 * Created 22.11.17 MLN
 * Last modified 22.11.17 MLN
 */
uint8_t* _timeToBCD(Time timeInput){
	uint8_t timeBCD[7] = {0};
	
	timeBCD[0] = ((timeInput.seconds / 10) << 4) + (timeInput.seconds % 10);
	timeBCD[1] = ((timeInput.minutes / 10) << 4) + (timeInput.minutes % 10);
	timeBCD[2] = ((timeInput.hours	 / 10) << 4) + (timeInput.hours   % 10);
	timeBCD[3] = timeInput.day;
	timeBCD[4] = ((timeInput.date	 / 10) << 4) + (timeInput.date    % 10);
	timeBCD[5] = (((timeInput.year - REFERENCE_YEAR) / 100) << 7) + ((timeInput.month / 10) << 4) + (timeInput.month % 10);
	timeBCD[6] = ((((timeInput.year - REFERENCE_YEAR) % 100) / 10) << 4) + (timeInput.year % 10);
	
	return timeBCD;
}



/* _BCDToTime
 *
 * Converts a BCD time to Time variable
 * Returns Time readable by software
 * NOT CHECKED YET
 *
 * Created 22.11.17 MLN
 * Last modified 22.11.17 MLN
 */
Time _BCDToTime (uint8_t* inputTable){
	Time returnTime;
	
	returnTime.seconds = 10 * (*(inputTable + 0) >> 4) + (*(inputTable + 0) & 0x0F);
	
	returnTime.minutes = 10 * (*(inputTable + 1) >> 4) + (*(inputTable + 1) & 0x0F);
	
	returnTime.hours = 10 * ((*(inputTable + 2) & 0x30) >> 4) + (*(inputTable + 2) & 0x0F);
	
	returnTime.day = *(inputTable + 3);
	
	returnTime.date	= 10 * (*(inputTable + 4) >> 4) + (*(inputTable + 4) & 0x0F);
	// Since a bit in the month register indicates the century, we compute years before months
	returnTime.year	= 100 * ((*(inputTable + 5) & 0x80) >> 7) * (*(inputTable + 6) >> 4) + (*(inputTable + 6) & 0x0F) + REFERENCE_YEAR;
	
	returnTime.month	= 10 * ((*(inputTable + 5) & 0x10) >> 4) + (*(inputTable + 5) & 0x0F);
	
	return returnTime;
}
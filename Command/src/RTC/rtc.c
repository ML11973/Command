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

bool timeChanged = false;
bool alarmReached = false;


/************************************************************************/
/* FUNCTIONS                                                            */
/************************************************************************/

void _timeToBCD(Time timeInput, uint8_t* timeBCD);
Time _BCDToTime (uint8_t* inputTable);

void _usartWrite(uint8_t *content, uint8_t contentSize);

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
 * Writes registers from firstRegister to firstRegister + dataNumber (non-included).
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
 * Last modified 24.11.17 MLN
 */
void rtc_setTime(void){
	twi_master_enable(RTC_TWI);
	_timeToBCD(currentTime, sentData);
	
	rtc_write(RTC_SECONDS, 7);
	twi_master_disable(RTC_TWI);
}



/* rtc_getTime
 *
 * TO BE MODIFIED, test function
 *
 * Created 15.11.17 MLN
 * Last modified 22.11.17 QVT
 */
void rtc_getTime(void){
	twi_master_enable(RTC_TWI);
	rtc_read(RTC_SECONDS, 7);
	
	currentTime = _BCDToTime(buffer);
	twi_master_disable(RTC_TWI);
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



/* rtc_setMinutesInterrupt
 *
 * Sets an interruption to occur on RTC ever minute.
 *
 * Created 22.11.17 QVT
 * Last modified 24.11.17 MLN
 */
void rtc_setMinutesInterrupt(){
	twi_master_enable(RTC_TWI);
	sentData[0] = 0x80; // Setting A2M2
	sentData[1] = 0x80; // Setting A2M3
	sentData[2] = 0x80; // Setting A2M4
	rtc_write(RTC_ALARM2_MINUTES,3);
	
	rtc_read(RTC_CONTROL,2);
	// Enabling interrupts on Alarm2
	sentData[0] = buffer[0] | (1<<A2IE) | (1<<INTCN);
	// Clearing Alarm2 interrupt flag
	sentData[1] = buffer[1] & ~(1<<A2F);
	rtc_write(RTC_CONTROL,2);
	twi_master_disable(RTC_TWI);
}



/* rtc_usart_sendTimeToDisplay
 *
 * Sends current time to display card via USART
 *
 * Created 22.11.17 QVT
 * Last modified 22.11.17 QVT
 */
void rtc_usart_sendTimeToDisplay(void){
	rtc_getTime();
	#ifndef CUSTOM_DATA_SENT_TO_DISPLAY
		uint8_t time[7];
		_timeToBCD(currentTime, time);
		uint8_t data[19] = {0xA5, 18, 0x80}; //start, length, useless data
		data[3] = 0x00; //mode
		data[4] = time[2];
		data[5] = time[1];
		data[6] = time[0];
		
		data[7]  = time[3];
		data[8]  = time[4];
		data[9]  = time[5];
		data[10] = time[6];
		
		for(uint8_t i = 0; i < 18; i++){
			data[18] ^= data[i];
		}
		
		_usartWrite(data, sizeof(data) / sizeof(data[0]));
		
	#else
	
	#endif
}



/* rtc_rtcISR
 *
 * Interruption triggered when RTC drives PA28 down.
 * RTC status register is read, flag status is extracted and
 * converted to software flags to use in main.
 *
 * Created 24.11.17 MLN
 * Last modified 24.11.17 MLN
 */
__attribute__((__interrupt__)) void rtc_rtcISR(void){
	twi_master_enable(RTC_TWI);
	
	// Reading RTC interrupt flag
	rtc_read(RTC_STATUS, 1);
	
	// Updating software alarm flags
	timeChanged = buffer[0] & 0x02;
	alarmReached = buffer[0] & 0x01;
	
	// Clearing RTC interrupt flags
	sentData[0] = buffer[0] & ~((timeChanged<<A2F) + (alarmReached<<A1F));
	 rtc_write(RTC_STATUS, 1);
	
	twi_master_disable(RTC_TWI);
	gpio_clear_pin_interrupt_flag(PIN_INT1);
}



/* _timeToBCD
 *
 * Converts a Time variable to BCD (format used by RTC).
 * Input table must be at least 7 bytes long
 *
 * Created 22.11.17 MLN
 * Last modified 22.11.17 QVT
 */
void _timeToBCD(Time timeInput, uint8_t* timeBCD){
	//uint8_t timeBCD[7] = {0};
	timeBCD[5] = (((timeInput.year) / 100) << 7) + ((timeInput.month / 10) << 4) + (timeInput.month % 10);
	timeBCD[6] = ((((timeInput.year) /10) % 10) << 4) + (timeInput.year % 10);
	
	timeBCD[0] = ((timeInput.seconds / 10) << 4) + (timeInput.seconds % 10);
	timeBCD[1] = ((timeInput.minutes / 10) << 4) + (timeInput.minutes % 10);
	timeBCD[2] = ((timeInput.hours	 / 10) << 4) + (timeInput.hours   % 10);
	timeBCD[3] = timeInput.day;
	timeBCD[4] = ((timeInput.date	 / 10) << 4) + (timeInput.date    % 10);
	
	//return timeBCD;
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
	returnTime.year	= 100 * ((*(inputTable + 5) & 0x80) >> 7) + 10 * (*(inputTable + 6) >> 4) + (*(inputTable + 6) & 0x0F);
	
	returnTime.month	= 10 * ((*(inputTable + 5) & 0x10) >> 4) + (*(inputTable + 5) & 0x0F);
	
	return returnTime;
}

void _usartWrite(uint8_t *content, uint8_t contentSize){
	while(contentSize > 0){
		if (usart_putchar(&AVR32_USART1,*content++) == USART_SUCCESS)
			contentSize--;
		else
			break;
	}
}




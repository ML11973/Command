/*
 * Screen.c
 *
 * Created: 06.11.2017 14:13:48
 *  Author: voletqu
 */ 
#include <asf.h>
#include "Screen.h"

#define SCREEN_WIDTH 					320
#define SCREEN_HEIGHT 					240

#define DISP_ORIENTATION			0			// Orientation vertical

#define LCD_ID_WRITE_INDEX			0x70
#define LCD_ID_READ_GRAM			0x73
#define LCD_ID_WRITE_GRAM			0x72

#define R61580_DEVICE_CODE			0x00		// Lecture du code 0x1580
#define R61580_NVM_CONTROL			0XA4		// all data in NVM is read and write
#define CALB						0

#define R61580_DRIVER_OUTPUT		0x60		// Driver output control

#define R61580_Y_CONTROL0			0x30		// y Control
#define R61580_Y_CONTROL1			0x31		// y Control
#define R61580_Y_CONTROL2			0x32		// y Control
#define R61580_Y_CONTROL3			0x33		// y Control
#define R61580_Y_CONTROL4			0x34		// y Control
#define R61580_Y_CONTROL5			0x35		// y Control
#define R61580_Y_CONTROL6			0x36		// y Control
#define R61580_Y_CONTROL7			0x37		// y Control
#define R61580_Y_CONTROL8			0x38		// y Control
#define R61580_Y_CONTROL9			0x39		// y Control

#define R61580_POWER_CONTROL1		0x10		// Setp-up Factor ant Output Voltage Level
#define R61580_POWER_CONTROL2		0x11		// Step-up clock frequency
#define R61580_POWER_CONTROL3		0x12		// Sets the factor to generate VREG1OUT
#define R61580_POWER_CONTROL4		0x13		// Set VCOM alternating amplitude in the range

#define R61580_DRIVER_CONTROL		0x01		// Sets shift direction
#define SS							8
#define SM							10
#define R61580_WAVE_CONTROL			0x02		// LCD Driving Wave
#define R61580_ENTRY_MODE			0x03		// Sets horizontal or Vertical direction
#define ORG							7
#define AM							3
#define ID0							4
#define ID1							5
#define BGR							12

#define R61580_DISPLAY_CONTROL1		0x07		//Display Control 1
#define R61580_DISPLAY_CONTROL2		0x08		//Display Control 2 (Nombre de ligne au dessus et au dessous)
#define R61580_DISPLAY_CONTROL3		0x09		//Scan Cycle
#define R61580_DISPLAY_CONTROL4		0x0A		//Sets output interval FMARK
#define R61580_INTERFACE_CONTROL1	0x0C		//Sets the interface format
#define R61580_MARKER_POSITION		0x0D		//Sets the output position

#define R61580_VCOM					0x0E		//Sets equalize function control bit
#define R61580_INTEFACE_CONTROL2	0x0F		//Sets the signal polarity

#define R61580_ADDRESS_HORIZONTAL	0x20
#define R61580_ADDRESS_VERTICAL		0x21		//A frame memory address Set
#define R61580_DATA_WRITE			0x22		//write operation

#define R61580_NVM_READ1			0x28		// The Data bits UID[7:0]
#define R61580_NVM_READ2			0x29		// Selects the factor of VREG1OUT
#define R61580_NVM_READ3			0x2A		//

#define R61580_ADD_HSA				0x50		// Adresse window Horizontal frame Address (Start Address)
#define R61580_ADD_HEA				0x51		// Adresse window Horizontal frame Address (End Address)
#define R61580_ADD_VEA				0x52		// Adresse window Vertical frame Address (Start Address)
#define R61580_ADD_VSA				0x52		// Adresse window Vertical frame Address (End Address)

#define R61580_BASE_IMAGE			0x61		// Base image Display Control
#define REV							0
#define VLE							1
#define NDL							2

#define R61580_SCROLL_VERTICAL		0x6A		//

#define R61580_PARTIAL_DISPLAY		0x80		// Sets the display position of partial image
#define R61580_PARTIAL_FRAME1		0x81		// Sets the start Line Address
#define R61580_PARTIAL_FRAME2		0x82		// Sets the end Line Address

#define R61580_PANEL_CONTROL1		0x90		// 80Hz
#define R61580_PANEL_CONTROL1_1		0x91		// Pre-charge period
#define R61580_PANEL_CONTROL2		0x92		// Sets the no overlap period
#define R61580_PANEL_CONTROL3		0x93		// Sets VCOM equalize period
#define R61580_PANEL_CONTROL4		0x94		// Defines source output alternating position within 1H period
#define R61580_PANEL_CONTROL5		0x95		// Sets RTNE and DIVE

#define Dummy						0xFF
#define SPI_SR_TDRE_MASK			2

static void _writeRegister(uint8_t reg, uint16_t data);
static uint16_t _readRegister(uint8_t reg);
static void _selectRegister(uint8_t address);

static void _writeRAM(uint16_t data);
static uint16_t _readRAM();

static void _setStart(Vector2 position);
static void _setLimits(Rectangle rect);

static void _powerUp();
static void _reset();
static void _setPWM();

void Screen_Init(){
	const gpio_map_t screenGPIO =
	{
		{PIN_SCK_SPI0,  FCT_SCK_SPI0 },			// SPI Clock.
		{PIN_MISO_SPI0, FCT_MISO_SPI0},			// MISO.
		{PIN_MOSI_SPI0, FCT_MOSI_SPI0},			// MOSI.
		{PIN_NPCS_TFT,  FCT_NPCS_TFT}			// Chip Select NPCS0 pour l'écran TFT.
	};

	spi_options_t screenOptions =
	{
		.reg			=	0,					// NPCS0 pour l'affichage.
		.baudrate		=   50000000,    		// Vitesse de transmission.
		.bits			= 	8,					// 8 bits de long.
		.spck_delay		= 	1,					// Delay entre CS et SPCK.
		.trans_delay	= 	0,					// Delay entre deux transfert.
		.stay_act		=	1,					// ?
		.spi_mode		= 	3,					// ?
		.modfdis		=	1					// ?
	};

	// Assign I/Os to SPI.
	gpio_enable_module(screenGPIO, sizeof(screenGPIO) / sizeof(screenGPIO[0]));
	// Initialize as master.
	spi_initMaster(AVR32_SPI0_ADDRESS, &screenOptions);
	// Enable SPI module.
	spi_enable(AVR32_SPI0_ADDRESS);
	// Initialize SD/MMC driver with SPI clock (PBA).
	spi_setupChipReg(AVR32_SPI0_ADDRESS, &screenOptions, BOARD_OSC0_HZ);

	_reset();
	_powerUp();
	_setPWM();
}


void Screen_SetBacklightStrength(uint8_t strength){


}

void Screen_TouchZone(Rectangle rect, void *callback(void)){


}

void Screen_SetPixel(Vector2 position, Color color){
	_setStart(position);
	_selectRegister(R61580_DATA_WRITE);
	_writeRAM(color.value);
	spi_unselectChip(SCREEN_SPI,SCREEN_SPI_NPCS);
}

void Screen_SetPixels(Rectangle rect, Color color){
	Rectangle_VerifySize(&rect, SCREEN_WIDTH, SCREEN_HEIGHT);

	_setLimits(rect);
	_setStart(rect.bottomLeft);

	_selectRegister(R61580_DATA_WRITE);
	volatile c = Rectangle_GetArea(rect);
	for(uint32_t count = c; count > 0; count--){
		_writeRAM(color.value);
	}

	spi_unselectChip(SCREEN_SPI, SCREEN_SPI_NPCS);
	
	_setLimits(Rect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT));
}

static void _writeRegister(uint8_t reg, uint16_t data){
	spi_selectChip(SCREEN_SPI, SCREEN_SPI_NPCS);		//Chip Select adresse du TFT.
	spi_write(SCREEN_SPI, LCD_ID_WRITE_INDEX);
	spi_write(SCREEN_SPI, 0x00);					// Adresse LSB
	spi_write(SCREEN_SPI, reg);					// Adresse MSB
	spi_unselectChip(SCREEN_SPI, SCREEN_SPI_NPCS);

	spi_selectChip(SCREEN_SPI, SCREEN_SPI_NPCS);		//Chip Select adresse du TFT.
	spi_write(SCREEN_SPI, LCD_ID_WRITE_GRAM);
	spi_write(SCREEN_SPI, (uint8_t) (data >> 8));
	spi_write(SCREEN_SPI, (uint8_t) (data & 0x00ff));
	spi_unselectChip(SCREEN_SPI, SCREEN_SPI_NPCS);
}

static uint16_t _readRegister(uint8_t reg){
	uint16_t data_low;
	uint16_t data_high;
	uint16_t data;
	spi_selectChip(SCREEN_SPI, SCREEN_SPI_NPCS);			//Chip Select adresse du TFT.
	spi_write(SCREEN_SPI, LCD_ID_WRITE_INDEX);
	spi_write(SCREEN_SPI, 0x00);						//Fixe l'adresse dans le registre d'Index.
	spi_write(SCREEN_SPI, reg);
	spi_unselectChip(SCREEN_SPI, SCREEN_SPI_NPCS);

	spi_selectChip(SCREEN_SPI, SCREEN_SPI_NPCS);			//Chip Select adresse du TFT.
	spi_write(SCREEN_SPI, LCD_ID_READ_GRAM);
	spi_write(SCREEN_SPI, Dummy);				// Write dummy
	spi_read(SCREEN_SPI, (uint16_t *) &data_high);
	spi_write(SCREEN_SPI, Dummy);
	spi_read(SCREEN_SPI, (uint16_t *) &data_low);
	spi_unselectChip(SCREEN_SPI, SCREEN_SPI_NPCS);

	data = data_low + (data_high << 8);
	return data;
}

static void _selectRegister(uint8_t address){
	spi_selectChip(SCREEN_SPI, SCREEN_SPI_NPCS);
	spi_write(SCREEN_SPI, LCD_ID_WRITE_INDEX);
	spi_write(SCREEN_SPI, 0x00);
	spi_write(SCREEN_SPI, address);
	spi_unselectChip(SCREEN_SPI, SCREEN_SPI_NPCS);
}

static void _powerUp(){
	uint16_t ID  = 0;
	
	//while (ID != 0x1580)
	while (ID != 0x0000)
	{
		ID = _readRegister(0x00);
	}
	
	// Synchronization after reset
	_writeRegister(R61580_DEVICE_CODE, 0x0000);
	_writeRegister(R61580_DEVICE_CODE, 0x0000);
	cpu_delay_ms(100, BOARD_OSC0_HZ);
	
	_writeRegister(R61580_DEVICE_CODE, 0x0000);
	_writeRegister(R61580_DEVICE_CODE, 0x0000);
	_writeRegister(R61580_DEVICE_CODE, 0x0000);
	_writeRegister(R61580_DEVICE_CODE, 0x0000);
	// Setup display
	_writeRegister(R61580_NVM_CONTROL, (uint16_t) (1 << CALB));	// CALB=1
	
	cpu_delay_ms(100, BOARD_OSC0_HZ);
	_writeRegister(R61580_DRIVER_OUTPUT, 0xA700);		// Driver Output Control
	_writeRegister(R61580_DISPLAY_CONTROL2, 0x0808);		// Display Control BP=8, FP=8
	
	//Gamma Setting:
	_writeRegister(R61580_Y_CONTROL0, 0x0203);		// y control
	_writeRegister(R61580_Y_CONTROL1, 0x080F);		// y control
	_writeRegister(R61580_Y_CONTROL2, 0x0401);		// y control
	_writeRegister(R61580_Y_CONTROL3, 0x050B);		// y control
	_writeRegister(R61580_Y_CONTROL4, 0x3330);		// y control
	_writeRegister(R61580_Y_CONTROL5, 0x0B05);		// y control
	_writeRegister(R61580_Y_CONTROL6, 0x0005);		// y control
	_writeRegister(R61580_Y_CONTROL7, 0x0F08);		// y control
	_writeRegister(R61580_Y_CONTROL8, 0x0302);		// y control
	_writeRegister(R61580_Y_CONTROL9, 0x3033);		// y control
	
	//Power Setting:
	
	_writeRegister(R61580_PANEL_CONTROL1, 0x0018);	// 80Hz
	_writeRegister(R61580_POWER_CONTROL1, 0x0530);	// Power Control
	_writeRegister(R61580_POWER_CONTROL2, 0x0237);
	_writeRegister(R61580_POWER_CONTROL3, 0x01BF);
	_writeRegister(R61580_POWER_CONTROL4, 0x1000);
	
	cpu_delay_ms(200, BOARD_OSC0_HZ);

	_writeRegister(R61580_DRIVER_CONTROL, 0x0000  | (1 << SS));
	_writeRegister(R61580_WAVE_CONTROL, 0x0200);
	//_write(R61580_ENTRY_MODE, 0x0000 | (1 << ORG) | (1 << ID0) | (1 << AM));
	_writeRegister(R61580_ENTRY_MODE, 0x0000 | (1 << ID1) | (1 << ID0));
	/*
	#if (DISP_ORIENTATION == 0)
	_write(R61580_ENTRY_MODE, 0x0000 | (1 << ID0) | (1 << AM));
	#else
	_write(R61580_ENTRY_MODE, 0x0000 | (1 << ID1) | (1 << ID0));
	#endif*/
	
	_writeRegister(R61580_DISPLAY_CONTROL3, 0x0001);		//Scan Cycle
	_writeRegister(R61580_DISPLAY_CONTROL4, 0x0008);		//Sets output interval FMARK
	_writeRegister(R61580_INTERFACE_CONTROL1, 0x0000);	//Sets the interface format
	_writeRegister(R61580_MARKER_POSITION, 0xD000);		//Sets the output position
	
	_writeRegister(R61580_VCOM, 0x0030);					//Sets equalize function control bit
	_writeRegister(R61580_INTEFACE_CONTROL2, 0x0000);	//Sets the signal polarity
	_writeRegister(R61580_ADDRESS_HORIZONTAL, 0);
	_writeRegister(R61580_ADDRESS_VERTICAL, 0);			//A frame memory address Set
	
	_writeRegister(R61580_NVM_READ2, 0x002E);			// Selects the factor of VREG1OUT 0x77
	
	_writeRegister(R61580_ADD_HSA, 0);				// Adresse window Horizontal frame Address (Start Address)
	_writeRegister(R61580_ADD_HEA, SCREEN_HEIGHT);		// Adresse window Horizontal frame Address (End Address)
	_writeRegister(R61580_ADD_VEA, 0);				// Adresse window Vertical frame Address (Start Address)
	_writeRegister(R61580_ADD_VSA, SCREEN_WIDTH);		// Adresse window Vertical frame Address (End Address)
	
	
	_writeRegister(R61580_BASE_IMAGE, (uint16_t) (1 << REV));	// Base image Display Control
	_writeRegister(R61580_SCROLL_VERTICAL, 0x0000);
	
	_writeRegister(R61580_PARTIAL_DISPLAY, 0x0000);		// Sets the display position of partial image
	_writeRegister(R61580_PARTIAL_FRAME1, 0x0000);		// Sets the start Line Address
	_writeRegister(R61580_PARTIAL_FRAME2, 0x005F);		// Sets the end Line Address
	_writeRegister(R61580_PANEL_CONTROL3, 0x0701);		// Sets VCOM equalize period
	
	_writeRegister(R61580_DISPLAY_CONTROL1, 0x0100);		//Display Control 1
}

static void _writeRAM(uint16_t data){
	spi_selectChip(SCREEN_SPI, SCREEN_SPI_NPCS);
	spi_write( SCREEN_SPI, LCD_ID_WRITE_GRAM);

	spi_write( SCREEN_SPI, (uint8_t)(data >> 8));
	spi_write( SCREEN_SPI, (uint8_t)(data & 0x00ff));
	spi_unselectChip(SCREEN_SPI, SCREEN_SPI_NPCS);
}

static void _setStart(Vector2 position){
	_writeRegister(R61580_ADDRESS_HORIZONTAL, position.y);
	_writeRegister(R61580_ADDRESS_VERTICAL	, position.x);
}

static void _setLimits(Rectangle rect){
	_writeRegister(R61580_ADD_HSA, rect.bottomLeft.y);
	_writeRegister(R61580_ADD_HEA, rect.topRight.y);
	_writeRegister(R61580_ADD_VSA, rect.bottomLeft.x);
	_writeRegister(R61580_ADD_VEA, rect.topRight.x);
}

static void _reset(){
	gpio_clr_gpio_pin(PIN_RESET_TFT);
	// 50us delay
	cpu_delay_ms( 100, BOARD_OSC0_HZ);

	gpio_set_gpio_pin(PIN_RESET_TFT);
	// 5ms delay
	cpu_delay_ms( 5, BOARD_OSC0_HZ);
}

static void _setPWM(){
	pwm_opt_t pwm_opt =
	{
		.diva = AVR32_PWM_DIVA_CLK_OFF,
		.divb = AVR32_PWM_DIVB_CLK_OFF,
		.prea = AVR32_PWM_PREA_MCK,
		.preb = AVR32_PWM_PREB_MCK
	};
		
	volatile avr32_pwm_channel_t pwm_channel = { .ccnt = 0 };

	// With these settings, the output waveform period will be:
	// (115200/256)/20 == 22.5Hz == (MCK/prescaler)/period, with
	// MCK == 115200Hz, prescaler == 256, period == 20. 

	pwm_channel.cdty = 1; // Channel duty cycle, should be < CPRD. 
	pwm_channel.cprd = 20; // Channel period. 
	pwm_channel.cupd = 0; // Channel update is not used here. 
	pwm_channel.CMR.calg = PWM_MODE_LEFT_ALIGNED; // Channel mode. 
	pwm_channel.CMR.cpol = PWM_POLARITY_LOW;      // Channel polarity. 
	pwm_channel.CMR.cpd = PWM_UPDATE_DUTY;        // Not used the first time. 
	pwm_channel.CMR.cpre = AVR32_PWM_CPRE_MCK_DIV_32; // Channel prescaler. 
	
	// Enable the alternative mode of the output pin to connect it to the PWM
	// module within the device. 
	gpio_enable_module_pin(PIN_PWM_TFT, FCT_PWM_TFT);

	// Initialize the PWM module. 
	pwm_init(&pwm_opt);

	// Set channel configuration to channel 0. 
	pwm_channel_init(5, &pwm_channel);

	// Start channel 0. 
	pwm_start_channels(1 << 5);
}
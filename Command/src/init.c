/**
 * \file
 *
 * \brief User board initialization template
 *
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */

#include "init.h"

#define TWI_MASTER_SPEED 100000
#define RTC_ADDRESS_WRITE 0xD0



void board_init(void) {
	sysclk_init();
	
	Disable_global_interrupt();
	// Configuration des vecteurs d'interruption:
	// Initialize and enable interrupt
	irq_initialize_vectors();
	cpu_irq_enable();
	
	twi_Init();
	usart_Init();
	spi1_Init();
	//tc0_Init();
	tc1_Init();
	dac_Init();
}


/* spi_Init
 *
 * Initializes SPI transmission for DAC volume control
 *
 * Created 06.11.17 MLN
 * Last modified 06.11.17 MLN
 */
void spi1_Init(void) {
	
	gpio_map_t SPI1_GPIO_MAP =
	{
		{ PIN_NPCS_SD,	FCT_NPCS_SD	  },
		{ PIN_NPCS_DA, FCT_NPCS_DA		},
		{ PIN_SCK_SPI1, FCT_SCK_SPI1   },
		{ PIN_MISO_SPI1, FCT_MISO_SPI1 },
		{ PIN_MOSI_SPI1, FCT_MOSI_SPI1 }
	};
	
	// DAC options for SPI communication
	spi_options_t spiOptionsDA2 =
	{
		.reg			=	2,					// NPCS2 pour le convertisseur DA
		.baudrate		=   1000000,   			// Vitesse de transmission.
		.bits			= 	16,					// 8 bits de long.
		.spck_delay		= 	0,					// Delay entre CS et SPCK.
		.trans_delay	= 	0,					// Delay entre deux transfert.
		.stay_act		=	1,					// ?
		.spi_mode		= 	1,					// CPOL = 0 / NCPHA = 0 => mode = 1
		.modfdis		=	1					// ?
	};
	
	spi_options_t spiOptionsSD = {
		.reg			=	1,			// NPCS1 pour la carte SD.
		.baudrate		=   1000000,   	// Vitesse de transmission.
		.bits			= 	8,			// 8 bits de long.
		.spck_delay		= 	0,			// Delay entre CS et SPCK.
		.trans_delay	= 	0,			// Delay entre deux transfert.
		.stay_act		=	1,			// ?
		.spi_mode		= 	0,			// ?
		.modfdis		=	1			// ?
	};
	
	// Assigning IOs to SPI
	gpio_enable_module(SPI1_GPIO_MAP, sizeof (SPI1_GPIO_MAP)/ sizeof(SPI1_GPIO_MAP[0]));
	
	// Initializing SPI as master
	spi_initMaster(SD_MMC_SPI, &spiOptionsSD);
	
	spi_selectionMode(SD_MMC_SPI, 0, 0, 0);
	// Initializes volume control DAC
	spi_setupChipReg(SD_MMC_SPI, &spiOptionsDA2, BOARD_OSC0_HZ);
	spi_setupChipReg(SD_MMC_SPI, &spiOptionsSD,  BOARD_OSC0_HZ);
	
	spi_enable(SD_MMC_SPI);
}



/* tc1_Init
 *
 * Initializes Timer/Counter 1 for 44.1 kHz interrupts.
 * Used as audio sample updater.
 *
 * Created 10.11.17 MLN
 * Last modified 11.11.17 MLN
 */
void tc1_Init(void){
	// Options for waveform generation.
	static const tc_waveform_opt_t WAVEFORM_OPT1 = {
		.channel  = TC1_CHANNEL,         // Canal selection.
		.bswtrg   = TC_EVT_EFFECT_NOOP,// Software trigger effect on TIOB.
		.beevt    = TC_EVT_EFFECT_NOOP,// External event effect on TIOB.
		.bcpc     = TC_EVT_EFFECT_NOOP,// RC compare effect on TIOB.
		.bcpb     = TC_EVT_EFFECT_NOOP,// RB compare effect on TIOB.
		.aswtrg   = TC_EVT_EFFECT_NOOP,// Software trigger effect on TIOA.
		.aeevt    = TC_EVT_EFFECT_NOOP,// External event effect on TIOA.
		.acpc     = TC_EVT_EFFECT_NOOP,// RC compare effect on TIOA: toggle.
		.acpa     = TC_EVT_EFFECT_NOOP,// RA compare effect on TIOA
		.wavsel   = TC_WAVEFORM_SEL_UPDOWN_MODE_RC_TRIGGER,// Up mode with automatic trigger
		.enetrg   = false,// External event trigger enable.
		.eevt     = TC_EXT_EVENT_SEL_TIOB_INPUT,// External event selection.
		.eevtedg  = TC_SEL_NO_EDGE,// External event edge selection.
		.cpcdis   = false,     // Counter disable when RC compare.
		.cpcstop  = false,     // Counter clock stopped with RC compare.

		.burst    = TC_BURST_NOT_GATED,     // Burst signal selection.
		.clki     = TC_CLOCK_RISING_EDGE,     // Clock inversion.
		.tcclks   = TC_CLOCK_SOURCE_TC2          // Int. source clock 1 connected to PBA/2
	};
	// 32MHz clock input PBA/2, output checked by scope measure
	
	static const tc_interrupt_t TC1_INTERRUPT =
	{
		.etrgs = 0, .ldrbs = 0, .ldras = 0, .cpcs  = 1,
		.cpbs  = 0, .cpas  = 0, .lovrs = 0, .covfs = 0
	};

	// Initialize the timer/counter.
	tc_init_waveform(&AVR32_TC, &WAVEFORM_OPT1);
	tc_write_rc(&AVR32_TC, TC1_CHANNEL, 181);  // Set RC value. 181 for 44.1 kHz sample rate
	tc_configure_interrupts(&AVR32_TC, TC1_CHANNEL, &TC1_INTERRUPT);
	INTC_register_interrupt (&tc1_irq, AVR32_TC_IRQ1, AVR32_INTC_INT0);
}



/* tc0_Init
 *
 * Initializes Timer/Counter 0 for 880 Hz interrupts.
 * Used as a square wave generator
 *
 * Created 10.11.17 MLN
 * Last modified 11.11.17 MLN
 */
void tc0_Init(void){
	// Options for waveform generation.
	static const tc_waveform_opt_t WAVEFORM_OPT0 = {
		.channel  = TC0_CHANNEL,         // Canal selection.
		.bswtrg   = TC_EVT_EFFECT_NOOP,// Software trigger effect on TIOB.
		.beevt    = TC_EVT_EFFECT_NOOP,// External event effect on TIOB.
		.bcpc     = TC_EVT_EFFECT_NOOP,// RC compare effect on TIOB.
		.bcpb     = TC_EVT_EFFECT_NOOP,// RB compare effect on TIOB.
		.aswtrg   = TC_EVT_EFFECT_NOOP,// Software trigger effect on TIOA.
		.aeevt    = TC_EVT_EFFECT_NOOP,// External event effect on TIOA.
		.acpc     = TC_EVT_EFFECT_NOOP,// RC compare effect on TIOA.
		.acpa     = TC_EVT_EFFECT_NOOP,// RA compare effect on TIOA.
		.wavsel   = TC_WAVEFORM_SEL_UP_MODE_RC_TRIGGER,// Up mode with automatic trigger
		.enetrg   = false,// External event trigger enable.
		.eevt     = TC_EXT_EVENT_SEL_TIOB_INPUT,// External event selection.
		.eevtedg  = TC_SEL_NO_EDGE,// External event edge selection.
		.cpcdis   = false,     // Counter disable when RC compare.
		.cpcstop  = false,     // Counter clock stopped with RC compare.

		.burst    = TC_BURST_NOT_GATED,     // Burst signal selection.
		.clki     = TC_CLOCK_RISING_EDGE,     // Clock inversion.
		.tcclks   = TC_CLOCK_SOURCE_TC3          // Int. source clock 1 connected to PBA/8
	};
	// 8 MHz clock connected to PBA/8
	
	static const tc_interrupt_t TC0_INTERRUPT =
	{
		.etrgs = 0, .ldrbs = 0, .ldras = 0, .cpcs  = 1,
		.cpbs  = 0, .cpas  = 0, .lovrs = 0, .covfs = 0
	};
	/*
	gpio_map_t T0_GPIO = {
		{PIN_T0_IOA, FCT_T0_IOA}
	};
	
	gpio_enable_module(T0_GPIO, 1);*/
	
	
	// Initialize the timer/counter.
	tc_init_waveform(&AVR32_TC, &WAVEFORM_OPT0);
	tc_write_rc(&AVR32_TC, TC0_CHANNEL, 36364);  // Set RC value for 880 Hz interrupt.
	tc_configure_interrupts(&AVR32_TC, TC0_CHANNEL, &TC0_INTERRUPT);
	INTC_register_interrupt (&tc0_irq, AVR32_TC_IRQ0, AVR32_INTC_INT1);
}



/* twi_Init
 *
 * Initializes Two-Wire Interface for communication with external RTC module
 *
 * Created 15.11.17 MLN
 * Last modified 15.11.17 MLN
 */
void twi_Init(void){
	gpio_set_gpio_pin(PIN_RTC_RST);
	gpio_set_gpio_pin(PIN_SDA);
	gpio_clr_gpio_pin(PIN_RTC_RST);
	// Master mode
	twi_options_t i2c_options =
	{
		.pba_hz = BOARD_OSC0_HZ,		// Vitesse du microcontr�leur 
		.speed = TWI_MASTER_SPEED,		// Vitesse de transmission 100-400KHz
		.chip = (RTC_ADDRESS_WRITE >> 1)// Adresse du slave
	};
	// Assign I/Os to TWI.
	const gpio_map_t TWI_GPIO_MAP =
	{
		{PIN_SCL, FCT_SCL},	// TWI Clock.
		{PIN_SDA, FCT_SDA}	// TWI Data.
	};
	gpio_enable_module(TWI_GPIO_MAP, sizeof(TWI_GPIO_MAP) / sizeof(TWI_GPIO_MAP[0]));
	twi_master_init(&AVR32_TWI, &i2c_options);
	
	// RTC alarm interruption initialization
	gpio_enable_pin_glitch_filter(PIN_INT1);
	gpio_enable_pin_pull_up(PIN_INT1);
	gpio_enable_pin_interrupt(PIN_INT1, GPIO_FALLING_EDGE);
	INTC_register_interrupt(&rtc_rtcISR, AVR32_GPIO_IRQ3, AVR32_INTC_INT0);
	twi_master_enable(&AVR32_TWI);
}



/* usart_Init()
 *
 * Initializes USART peripheral for RTC data transmission to display card.
 *
 * Created 22.11.17 QVT
 * Last modified 22.11.17 QVT
 */
void usart_Init(void){
	const usart_options_t USART_OPTIONS =
	{
		.baudrate     = 250000,
		.charlength   = 8,
		.paritytype   = USART_NO_PARITY,
		.stopbits     = USART_1_STOPBIT,
		.channelmode  = USART_NORMAL_CHMODE
	};
	
	const gpio_map_t USART_GPIO_MAP =
	{
		{USART1_TXD_PIN, USART1_TXD_FCT},
		{USART1_RXD_PIN, USART1_RXD_FCT},
		{USART1_CLK_PIN, USART1_CLK_FCT},
		{USART1_RTS_PIN, USART1_RTS_FCT},
		{USART1_CTS_PIN, USART1_CTS_FCT}
	};
	
	usart_init_rs232(&AVR32_USART1, &USART_OPTIONS, (BOARD_OSC0_HZ / 2));
	gpio_enable_module (USART_GPIO_MAP,sizeof(USART_GPIO_MAP)/sizeof(USART_GPIO_MAP[0]));
}


/* dac_Init()
 *
 * Initializes DAC module by assigning required pins as outputs
 * and setting the pins required for its functioning.
 *
 * Created 22.11.17 MLN
 * Last modified 22.11.17 MLN
 */
void dac_Init(void)
{
	// Configuring PB0-15 for audio output
	AVR32_GPIO.port[1].oders = 0x0000FFFF;
	
	gpio_set_gpio_pin(PIN_SHUTDOWN);
	gpio_set_gpio_pin(DAC_CS_PIN);			// Chip select inactive = 1
	gpio_set_gpio_pin(DAC_WR_PIN);			// Write active low
	gpio_set_gpio_pin(DAC_LDAC_PIN);		// Transfer active low
	gpio_set_gpio_pin(DAC_PD_PIN);
	
	// Deleting DAC contents
	gpio_set_gpio_pin(DAC_CLR_PIN);
	gpio_clr_gpio_pin(DAC_CLR_PIN);
	gpio_set_gpio_pin(DAC_CLR_PIN);
}
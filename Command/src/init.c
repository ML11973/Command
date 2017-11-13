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




void board_init(void) {
	sysclk_init();
	
	Disable_global_interrupt();
	// Configuration des vecteurs d'interruption:
	// Initialize and enable interrupt
	irq_initialize_vectors();
	cpu_irq_enable();
	
	spi_init();
	//tc0_init();
	tc1_init();
	// Configuring PB0-15
	gpio_configure_group(1, 0x0000FFFF, GPIO_DIR_OUTPUT);
}



void spi_init(void) {
	
	gpio_map_t DA2_SPI1_GPIO_MAP =
	{
		{ PIN_NPCS_DA, FCT_NPCS_DA },
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
	
	// Assigning IOs to SPI
	gpio_enable_module(DA2_SPI1_GPIO_MAP, sizeof (DA2_SPI1_GPIO_MAP)/ sizeof(DA2_SPI1_GPIO_MAP[0]));
	
	// Initializing SPI as master
	spi_initMaster((volatile avr32_spi_t*)AVR32_SPI1_ADDRESS, &spiOptionsDA2);
	
	// Initializes volume control DAC
	spi_setupChipReg((volatile avr32_spi_t*)AVR32_SPI1_ADDRESS, &spiOptionsDA2, BOARD_OSC0_HZ);
	
	spi_enable((volatile avr32_spi_t*)AVR32_SPI1_ADDRESS);
	
}


// 32MHz clock input PBA/2, output checked by scope measure
void tc1_init(void){
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
	
	static const tc_interrupt_t TC1_INTERRUPT =
	{
		.etrgs = 0, .ldrbs = 0, .ldras = 0, .cpcs  = 1,
		.cpbs  = 0, .cpas  = 0, .lovrs = 0, .covfs = 0
	};
	/*
	static const gpio_map_t T1_GPIO = {
		{PIN_T1_IOA, FCT_T1_IOA}
	};
	
	gpio_enable_module(T1_GPIO, 1);
	*/
	
	// Initialize the timer/counter.
	tc_init_waveform(&AVR32_TC, &WAVEFORM_OPT1);
	tc_write_rc(&AVR32_TC, TC1_CHANNEL, 181);  // Set RC value.
	tc_configure_interrupts(&AVR32_TC, TC1_CHANNEL, &TC1_INTERRUPT);
	INTC_register_interrupt (&tc1_irq, AVR32_TC_IRQ1, AVR32_INTC_INT0);
	tc_start(&AVR32_TC, TC1_CHANNEL);
}


// 8 MHz clock connected to PBA/8
void tc0_init(void){
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


// RTC TWI configuration
void twi_init(void){
	
}
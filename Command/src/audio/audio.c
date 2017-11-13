/*
 * audio.c
 *
 * Created: 06.11.2017 13:49:49
 *  Author: leemannma
 */ 

#include "audio.h"

#ifndef BOARD_OSC0_HZ
	#define BOARD_OSC0_HZ 64000000
#endif

#define AUDIOOUTPUTMASK 0x000003FF
#define AUDIOMAXINPUT 0x3FF

uint16_t sineTable [] = {
	0x200,0x20f,0x21f,0x22e,0x23e,0x24d,0x25d,0x26c,
	0x27b,0x28a,0x299,0x2a8,0x2b7,0x2c5,0x2d3,0x2e1,
	0x2ef,0x2fd,0x30a,0x318,0x325,0x331,0x33e,0x34a,
	0x355,0x361,0x36c,0x377,0x381,0x38b,0x395,0x39e,
	0x3a7,0x3b0,0x3b8,0x3c0,0x3c7,0x3ce,0x3d4,0x3da,
	0x3e0,0x3e5,0x3ea,0x3ee,0x3f2,0x3f5,0x3f8,0x3fa,
	0x3fc,0x3fe,0x3fe,0x3ff,0x3ff,0x3fe,0x3fe,0x3fc,
	0x3fa,0x3f8,0x3f5,0x3f2,0x3ee,0x3ea,0x3e5,0x3e0,
	0x3da,0x3d4,0x3ce,0x3c7,0x3c0,0x3b8,0x3b0,0x3a7,
	0x39e,0x395,0x38b,0x381,0x377,0x36c,0x361,0x355,
	0x34a,0x33e,0x331,0x325,0x318,0x30a,0x2fd,0x2ef,
	0x2e1,0x2d3,0x2c5,0x2b7,0x2a8,0x299,0x28a,0x27b,
	0x26c,0x25d,0x24d,0x23e,0x22e,0x21f,0x20f,0x200,
	0x1f0,0x1e0,0x1d1,0x1c1,0x1b2,0x1a2,0x193,0x184,
	0x175,0x166,0x157,0x148,0x13a,0x12c,0x11e,0x110,
	0x102,0xf5,0xe7,0xda,0xce,0xc1,0xb5,0xaa,
	0x9e,0x93,0x88,0x7e,0x74,0x6a,0x61,0x58,
	0x4f,0x47,0x3f,0x38,0x31,0x2b,0x25,0x1f,
	0x1a,0x15,0x11,0xd,0xa,0x7,0x5,0x3,
	0x1,0x1,0x0,0x0,0x1,0x1,0x3,0x5,
	0x7,0xa,0xd,0x11,0x15,0x1a,0x1f,0x25,
	0x2b,0x31,0x38,0x3f,0x47,0x4f,0x58,0x61,
	0x6a,0x74,0x7e,0x88,0x93,0x9e,0xaa,0xb5,
	0xc1,0xce,0xda,0xe7,0xf5,0x102,0x110,0x11e,
	0x12c,0x13a,0x148,0x157,0x166,0x175,0x184,0x193,
0x1a2,0x1b2,0x1c1,0x1d1,0x1e0,0x1f0,0x200};

uint16_t i = 0;
uint16_t j = 0;
uint16_t audioR = 0;
uint16_t audioL = 0;

/*
 * set_volume
 *
 * This function sends 2 bytes to N4: AD5300BRMZ to set its analog output.
 * The analog output sets in turn the volume of the N7: TPA6012A4 audio
 * amplifier.
 * 
 * Created 06.11.17 MLN
 * Last modified 13.11.17 MLN
 */

void audio_set_volume (uint8_t volume){
	
	// Selecting DAC1
	spi_selectChip((volatile struct avr32_spi_t*)DAC1_SPI, DAC1_SPI_NPCS);
	
	/* Sending volume level to DAC1
	 * Volume has to be in the middle nibbles of a 2-byte integer
	 * as per AD5300BRMZ datasheet
	 */
	spi_write((volatile struct avr32_spi_t*)DAC1_SPI, volume<<4);
	
	// Deselecting DAC1
	spi_unselectChip((volatile struct avr32_spi_t*)DAC1_SPI, DAC1_SPI_NPCS);
	
	if (volume == 0) {
		// If volume superior to 0, do not shutdown amplifier
		gpio_clr_gpio_pin(PIN_SHUTDOWN);
	} else {
		gpio_set_gpio_pin(PIN_SHUTDOWN);
	}
}



/* audio_set_output (uint16_t input)
 *
 * Sets N5: AD5333BRUZ to output input argument on both channels.
 * Input must be 10-bit, right-aligned
 * Timing specifications can be found in AD5333BRUZ datasheet, page 3.
 * 	
 * Channel A corresponds to the right speaker
 * Channel B corresponds to the left speaker 
 *
 * Created 06.11.17 MLN
 * Last modified 08.11.17 MLN
 */

void audio_set_output (uint16_t inputA, uint16_t inputB){
	
	// First we update DA0-9 parallel inputs
	
	// Output is masked to only affect PB0-9 in case of input error
	uint32_t output = inputA & AUDIOOUTPUTMASK;
	// PB0-9 driven low
	gpio_set_group_low(PORTB, AUDIOOUTPUTMASK);
	// PB0-9 assigned output value
	gpio_set_group_high(PORTB, output);
	// B Output value is computed before DAC refresh
	output = inputB & AUDIOOUTPUTMASK;
	
	
	
	// Then we enable the DAC by setting active-low clear and power-down
	gpio_set_gpio_pin(DAC_PD_PIN);
	gpio_set_gpio_pin(DAC_CLR_PIN);
	
	
	
	// Then we load the data in DAC channel A
	
	// Selecting chip
	gpio_clr_gpio_pin(DAC_CS_PIN);
	gpio_clr_gpio_pin(DAC_WR_PIN);
	// Selecting channel A
	gpio_set_gpio_pin(DAC_A0_PIN);
	// Loading data in Channel A
	gpio_clr_gpio_pin(DAC_LDAC_PIN);
	// De-selecting chip
	gpio_set_gpio_pin(DAC_WR_PIN);
	gpio_set_gpio_pin(DAC_CS_PIN);
	
	
	
	// We update the parallel input
	
	// PB0-9 driven low
	gpio_set_group_low(PORTB, AUDIOOUTPUTMASK);
	// PB0-9 assigned output value
	gpio_set_group_high(PORTB, output);
	
	
	
	// We load input in channel B
	
	// Selecting channel B
	gpio_clr_gpio_pin(DAC_A0_PIN);
	// Selecting chip
	gpio_clr_gpio_pin(DAC_CS_PIN);
	gpio_clr_gpio_pin(DAC_WR_PIN);
	// Data is loaded in channel B
	// De-selecting chip
	gpio_set_gpio_pin(DAC_WR_PIN);
	gpio_set_gpio_pin(DAC_CS_PIN);
	
	
	
	// Final refresh
	gpio_set_gpio_pin(DAC_LDAC_PIN);
}




void freq_start (uint16_t freq){
	// Writes value to RC such that interrupt occurs at 2*freq Hz
	tc_write_rc(&AVR32_TC, TC0_CHANNEL, (BOARD_OSC0_HZ/4)*freq);
	// Starts timer0
	tc_start(&AVR32_TC, TC0_CHANNEL);
}




void freq_stop (void){
	tc_stop(&AVR32_TC, TC0_CHANNEL);
}




/* Timer 1 interruption
 *
 * This interruption occurs at 44.1 kHz for .wav playback
 *
 */
__attribute__((__interrupt__)) void tc1_irq( void ){
	audio_set_output(audioR, audioL);
}



__attribute__((__interrupt__)) void tc0_irq( void ){
	if (audioR == 0 || audioL == 0)
	{
		audioR = AUDIOMAXINPUT;
		audioL = AUDIOMAXINPUT;
	}
	else
	{
		audioR = 0;
		audioL = 0;
	}
	audio_set_output(audioR, audioL);
}
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

/************************************************************************/
/* DEFINITIONS                                                          */
/************************************************************************/

#define AUDIOOUTPUTMASK 0x000003FF
#define AUDIOMAXINPUT 0x3FF



/************************************************************************/
/* VARIABLES                                                            */
/************************************************************************/
const uint16_t sineTable [] = {
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
	0x102,0x0f5,0x0e7,0x0da,0x0ce,0x0c1,0x0b5,0x0aa,
	0x09e,0x093,0x088,0x07e,0x074,0x06a,0x061,0x058,
	0x04f,0x047,0x03f,0x038,0x031,0x02b,0x025,0x01f,
	0x01a,0x015,0x011,0x00d,0x00a,0x007,0x005,0x003,
	0x001,0x001,0x000,0x000,0x001,0x001,0x003,0x005,
	0x007,0x00a,0x00d,0x011,0x015,0x01a,0x01f,0x005,
	0x02b,0x031,0x038,0x03f,0x047,0x04f,0x058,0x061,
	0x06a,0x074,0x07e,0x088,0x093,0x09e,0x0aa,0x0b5,
	0x0c1,0x0ce,0x0da,0x0e7,0x0f5,0x102,0x110,0x11e,
	0x12c,0x13a,0x148,0x157,0x166,0x175,0x184,0x193,
	0x1a2,0x1b2,0x1c1,0x1d1,0x1e0,0x1f0,0x200};

uint16_t i = 0;
uint16_t j = 0;
uint16_t audioR = 0;
uint16_t audioL = 0;
bool nextSample = 0;
uint8_t volume = DEFAULTVOLUME;

volatile uint32_t ram = 0;

uint16_t wavDataIndex = 0;

uint8_t wavData1[512] = {0};
uint8_t wavData2[512] = {0};

AudioInfo fileData;



/************************************************************************/
/* FONCTIONS                                                            */
/************************************************************************/

uint8_t _fileVerification();


/* audio_setVolume
 *
 * This function sends 2 bytes to N4: AD5300BRMZ to set its analog output.
 * The analog output sets in turn the volume of the N7: TPA6012A4 audio
 * amplifier.
 * 
 * Created 06.11.17 MLN
 * Last modified 13.11.17 MLN
 */

void audio_setVolume (uint8_t volume){
	
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

void _setOutput (uint16_t inputA, uint16_t inputB){
	
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




void audio_freqStart (uint16_t freq){
	// Writes value to RC such that interrupt occurs at 2*freq Hz
	tc_write_rc(&AVR32_TC, TC0_CHANNEL, (BOARD_OSC0_HZ/8)/(2*freq) );
	// Starts timer0
	tc_start(&AVR32_TC, TC0_CHANNEL);
}




void audio_freqStop (void){
	tc_stop(&AVR32_TC, TC0_CHANNEL);
}




/* Timer 1 interruption
 *
 * This interruption occurs at 44.1 kHz for .wav playback
 *
 * Created 09.11.17 MLN
 * Last modified 17.11.17 MLN
 */
__attribute__((__interrupt__)) void tc1_irq( void ){
	_setOutput(audioR, audioL);
	nextSample = 1;
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
	_setOutput(audioR, audioL);
}



/* audio_playFile
 *
 * Plays a file from the SD card. Must be put in while(1) loop.
 * Loads file first sector, puts file info in a global variable
 * and loads next sector when finished sector flag is raised from
 * timer 1 interruption.
 *
 * Created 17.11.17 MLN
 * Last modified 17.11.17 MLN
 */
uint8_t audio_playFile(uint8_t fileNumber){
	static uint32_t sampleCounter = 0;
	static uint8_t currentVolume = DEFAULTVOLUME;
	static audio_firstCall = 1;
	
	
	// If called upon for the first time, initalize .wav reading
	if (audio_firstCall == true){
		// If no SD card, return specific error code
		if (sdcard_mount() == false){
			return ERROR_NO_SD;
		}
		// If no file, return specific error code
		if (sdcard_setFileToRead(fileNumber) == false){
			return ERROR_NO_FILE;
		}
		
		sdcard_getNextSector(wavData1);
		sdcard_getNextSector(wavData2);
		
		if ( _fileVerification(wavData1) == true ){
			audio_firstCall = 0;
		}
		
		
	}
	
	// Volume setting, optimized to be as light as possible
	if (currentVolume != volume){
		currentVolume = volume;
		audio_setVolume(currentVolume);
	}
	
	if (nextSample == true){
		// Update audioL and audioR values accordingly
		// Raise tc0 update flag
	}
	
	
}



/* _fileVerification
 *
 * http://soundfile.sapp.org/doc/WaveFormat/
 * http://www-mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html more detailed
 * Checks validity of audio parameters and loads relevant ones in fileData.
 * Returns program-wide error codes as defined in audio.h
 *
 * Created 17.11.17 MLN
 * Last modified 17.11.17 MLN
 */
uint8_t _fileVerification(){
	uint16_t headerIndex = 8;
	// Verifying file format ID in RIFF header
	if (wavData1[headerIndex] != 'W' || wavData1[headerIndex + 1] != 'A' || wavData1[headerIndex + 2] != 'V' || wavData1[headerIndex + 3] != 'E'){
		return ERROR_FORMAT;
	}
	
	
	// Beginning fetch AudioInfo routine
	
	// Finding "fmt " subchunk in header, then jumping to next relevant block
	while (wavData1[headerIndex] != 'f' && wavData1[headerIndex + 1] != 'm' && wavData1[headerIndex + 2] != 't' && wavData1[headerIndex + 3] != ' ') {
		headerIndex++;
		if (headerIndex > SECTOR_SIZE){
			return ERROR_NO_FMT_SUBCHUNK;
		}
	}
	// Jumps to Subchuk1Size block
	headerIndex += 4;
	// Jumps to AudioFormat block
	headerIndex += 4;
	
	if (wavData1[headerIndex + 1] != 1) {
		return ERROR_FILE_COMPRESSED;
	}
	
	// Jumps to NumChannels block
	headerIndex += 2;
	fileData.channelNumber = wavData1[headerIndex + 1];
	
	// Jumps to SampleRate block
	headerIndex += 2;
	// Puts 4 SampleRate bytes on a single 32-bit integer before assigning it to fileData.sampleRate
	fileData.sampleRate = 
		(wavData1[headerIndex] << 24) |
		(wavData1[headerIndex + 1] << 16) |
		(wavData1[headerIndex + 2] << 8) |
		(wavData1[headerIndex + 3])
	;
	
	// Jumps to ByteRate block
	headerIndex += 4;
	
	// Jumps to BlockAlign block
	headerIndex += 4;
	// Assigns fileData.blockAlign value
	fileData.blockAlign = 
		(wavData1[headerIndex] << 8) |
		(wavData1[headerIndex + 1])
	;
	
	// Jumps to BitsPerSample block
	headerIndex += 2;
	fileData.bitsPerSample =
		(wavData1[headerIndex] << 8) |
		(wavData1[headerIndex + 1])
	;
	
	// Finding "data" chunk
	while (wavData1[headerIndex] != 'f' && wavData1[headerIndex + 1] != 'm' && wavData1[headerIndex + 2] != 't' && wavData1[headerIndex + 3] != ' ') {
		headerIndex++;
		if (headerIndex > SECTOR_SIZE){
			return ERROR_NO_DATA_SUBCHUNK;
		}
	}
	
	// Jumps to Subchunk2Size
	headerIndex += 4;
	fileData.audioSampleBytes =
		(wavData1[headerIndex] << 24) |
		(wavData1[headerIndex + 1] << 16) |
		(wavData1[headerIndex + 2] << 8) |
		(wavData1[headerIndex + 3])
	;
	
	// Jumps to Data
	headerIndex += 4;
	fileData.firstDataByteIndex = headerIndex;
	
	// Ending fetch AudioInfo routine
	
	
	// Verifying fileData parameters
	if (
		(fileData.channelNumber != AUDIO_CHANNELS) ||
		(fileData.sampleRate != AUDIO_SAMPLERATE) ||
		(fileData.blockAlign != AUDIO_BLOCKALIGN) ||
		(fileData.bitsPerSample != AUDIO_BPS)
	) {
		return ERROR_INCOMPATIBLE_FILE;
	}
	return true;
}
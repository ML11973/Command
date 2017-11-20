/*
 * audio.c
 *
 * Created: 06.11.2017 13:49:49
 *  Author: MLN
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
#define WAVDATA_LAST_INDEX 511



/************************************************************************/
/* VARIABLES                                                            */
/************************************************************************/

uint16_t audioR = 0;
uint16_t audioL = 0;
bool loadNextSample = 0;
uint8_t currentVolume = DEFAULTVOLUME;

uint8_t wavData1[512] = {0};
uint8_t wavData2[512] = {0};


AudioInfo fileData;



/************************************************************************/
/* FUNCTIONS                                                            */
/************************************************************************/

void _setOutput (uint16_t, uint16_t);
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
	
	if (volume != currentVolume){
		currentVolume = volume;
		// Selecting DAC1
		spi_selectChip((volatile struct avr32_spi_t*)DAC1_SPI, DAC1_SPI_NPCS);
	
		/* Sending volume level to DAC1
		 * Volume has to be in the middle nibbles of a 2-byte integer
		 * as per AD5300BRMZ datasheet
		 */
		spi_write((volatile struct avr32_spi_t*)DAC1_SPI, volume<<4);
	
		// Deselecting DAC1
		spi_unselectChip((volatile struct avr32_spi_t*)DAC1_SPI, DAC1_SPI_NPCS);
	
		if (currentVolume == 0) {
			// If volume superior to 0, do not shutdown amplifier
			gpio_clr_gpio_pin(PIN_SHUTDOWN);
		} else {
			gpio_set_gpio_pin(PIN_SHUTDOWN);
		}
	}
}



/* audio_playFile
 *
 * Plays a file from the SD card. Must be put in while(1) loop.
 * Loads file first sector, puts file info in a global variable
 * and loads next sector when finished sector flag is raised from
 * timer 1 interruption.
 *
 * Created 17.11.17 MLN
 * Last modified 20.11.17 MLN
 */
uint8_t audio_playFile(uint8_t fileNumber){
	static audio_firstCall = true;
	static uint8_t *wavDataPointer = wavData1;
	static uint32_t wavDataIndex = 0;
	
	
	
	/************************************************************************/
	/* INITIALIZATION														*/
	/* Mounts the sdcard, sets it to read the selected file from its index, */
	/* loads the first two sectors in the wavData arrays, checks its format */
	/* and completes its infos and starts the 44.1 kHz timer used for audio */
	/* output timing.														*/
	/************************************************************************/
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
			audio_firstCall = false;
		}
		
		tc_start(&AVR32_TC, TC1_CHANNEL);
	}
	
	
	
	/************************************************************************/
	/* FILE READING ROUTINE													*/
	/* This segment sets the volume if it has been changed, loads the 4		*/
	/* bytes representing the sample, puts them on 10 bits and sends them	*/
	/* to the output stage													*/
	/************************************************************************/
	
	// Volume setting, optimized to be as light as possible
	audio_setVolume(currentVolume);
	
	
	if (loadNextSample == true){
		/* Update audioL and audioR values using pointer to array values
		 * Audio values must be 10-bit.
		 * Shifting through the array by incrementing wavDataIndex.
		*/
		audioL = *(wavDataPointer + wavDataIndex % WAVDATA_LAST_INDEX);
		audioL <<= 2;
		wavDataIndex++;
		audioL |= *(wavDataPointer + (wavDataIndex % WAVDATA_LAST_INDEX) + 1) >> 6;
		wavDataIndex++;
		
		
		audioR = *(wavDataPointer + (wavDataIndex % WAVDATA_LAST_INDEX) + 2);
		audioR <<= 2;
		wavDataIndex++;
		audioR |= *(wavDataPointer + (wavDataIndex % WAVDATA_LAST_INDEX) + 3) >> 6;
		wavDataIndex++;
		
		// If pointer reaches the end of the current wavData array
		if ( wavDataIndex % WAVDATA_LAST_INDEX == 0){
			
			// Switching between arrays then
			// loading next sector in previously used array
			if (wavDataPointer == wavData1){
				wavDataPointer = wavData2;
				sdcard_getNextSector(wavData1);
			}
			else {
				wavDataPointer = wavData1;
				sdcard_getNextSector(wavData2);
			}
		}
		
		// Lowering audio update flag
		loadNextSample = false;
	}
	
	
	
	/************************************************************************/
	/* END OF FILE ROUTINE													*/
	/* Stops audio playback by stopping the timer and returns a finish code	*/
	/************************************************************************/
	
	if (wavDataIndex >= fileData.audioSampleBytes){
		tc_stop(&AVR32_TC, TC1_CHANNEL);
		
		audio_firstCall = true;
		loadNextSample = true;
		wavDataIndex = 0;
		
		return AUDIO_PLAY_FINISHED;
	}
	
}



/* audio_freqStart
 *
 * Starts tc0 to generate a square wave at argument frequency.
 *
 * Created 08.11.17 MLN
 * Last modified 10.11.17 MLN
 */
void audio_freqStart (uint16_t freq){
	// Writes value to RC such that interrupt occurs at 2*freq Hz
	tc_write_rc(&AVR32_TC, TC0_CHANNEL, (BOARD_OSC0_HZ/8)/(2*freq) );
	// Starts timer0
	tc_start(&AVR32_TC, TC0_CHANNEL);
}



/* audio_freqStop
 *
 * Stops previously started tc0.
 *
 * Created 08.11.17 MLN
 * Last modified 10.11.17 MLN
 */
void audio_freqStop (void){
	tc_stop(&AVR32_TC, TC0_CHANNEL);
}



/* _setOutput
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



/* Timer 1 interruption
 *
 * This interruption occurs at 44.1 kHz for .wav playback
 * Uses nextSample flag to tell audio_playFile to load
 * the next sample, and so that tc1 doesn't refresh
 * audio output if it has not changed since last interruption
 *
 * Created 09.11.17 MLN
 * Last modified 20.11.17 MLN
 */
__attribute__((__interrupt__)) void tc1_irq( void ){
	if (loadNextSample == false){
		_setOutput(audioR, audioL);
		loadNextSample = true;
	}
}



/* Timer 0 interruption
 *
 * This interruption generates a square wave on audio output.
 * CURRENTLY NOT WORKING, SEE TIMER INITIALIZATION
 *
 * Created 09.11.17 MLN
 * Last modified 13.11.17 MLN
 */
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

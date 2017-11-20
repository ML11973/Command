/*
 * audio.h
 *
 * Created: 06.11.2017 13:52:58
 *  Author: leemannma
 */ 

#ifndef AUDIO_H
#define AUDIO_H

/************************************************************************/
/* INCLUDES                                                             */
/************************************************************************/

#include <asf.h>
#include "SDCard/sdcard.h"



/************************************************************************/
/* DEFINITIONS                                                          */
/************************************************************************/

// Default volume
#define DEFAULTVOLUME 0x40

// Default required .wav parameters
#define AUDIO_SAMPLERATE	44100
#define AUDIO_CHANNELS		2
#define AUDIO_BPS			16
#define AUDIO_BLOCKALIGN	AUDIO_CHANNELS * AUDIO_BPS / 8

// Audio playback error codes
#define ERROR_NO_SD					0x02
#define ERROR_NO_FILE				0x03
#define ERROR_FORMAT				0x04
#define ERROR_FILE_COMPRESSED		0x05
#define ERROR_NO_FMT_SUBCHUNK		0x06
#define ERROR_NO_DATA_SUBCHUNK		0x07
#define ERROR_INCOMPATIBLE_FILE		0x08

typedef struct audioInfo {
	uint8_t channelNumber;		// Number of audio channels (2 for stereo)
	uint32_t sampleRate;		// Number of audio samples per second (44100)
	uint16_t blockAlign;		// Number of bytes per sample per channel
	uint16_t bitsPerSample;		// Number of bits per sample
	uint32_t audioSampleBytes;	// Number of bytes to read
	
	uint8_t firstDataByteIndex;	// Index of first data block in file
}AudioInfo;


/************************************************************************/
/* VARIABLES                                                            */
/************************************************************************/
extern const uint16_t sineTable[];
extern uint16_t i;
extern uint16_t j;
extern uint8_t max;
extern uint16_t audioL;
extern uint16_t audioR;
extern volatile uint32_t ram;
extern uint8_t volume;
extern AudioInfo fileData;


/************************************************************************/
/* FUNCTIONS	                                                        */
/************************************************************************/


/* set_volume
 *
 * This function sends 2 bytes to N4: AD5300BRMZ to set its analog output.
 * The analog output sets in turn the volume of the N7: TPA6012A4 audio
 * amplifier.
 * 
 * Created 06.11.17 MLN
 * Last modified 08.11.17 MLN
 */
void audio_setVolume (uint8_t);



/* audio_set_output
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
void _setOutput (uint16_t, uint16_t);



void audio_freqStart(uint16_t);
void audio_freqStop(void);

uint8_t audio_playFile(uint8_t fileNumber);

/* Timer 1 interruption
 *
 * This interruption occurs at 44.1 kHz for .wav playback
 *
 */
void tc1_irq(void);



void tc0_irq(void);

#endif /* AUDIO_H */
/*
 * audio.h
 *
 * Created: 06.11.2017 13:52:58
 *  Author: MLN
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
#define AUDIO_PLAYING				0x09	// Audio is being played
#define AUDIO_PLAY_FINISHED			0x01	// Reached end of file
#define FILE_VERIFICATION_SUCCEDED	0x01	//
#define ERROR_NO_SD					0x02	// No SD Card detected
#define ERROR_NO_FILE				0x03	// Selected file not found
#define ERROR_FORMAT				0x04	// File format is not .wav
#define ERROR_FILE_COMPRESSED		0x05	// File is compressed (not supported)
#define ERROR_NO_FMT_SUBCHUNK		0x06	// Couldn't find Format subchunk (abort)
#define ERROR_NO_DATA_SUBCHUNK		0x07	// Couldn't find audio Data subchunk (abort)
#define ERROR_INCOMPATIBLE_FILE		0x08	// File parameters incompatible with basic 
											// audio playback program. See line 28 above

#define MAXMETADATACHARS			24



typedef struct audioInfo {
	uint8_t channelNumber;		// Number of audio channels (2 for stereo)
	uint32_t sampleRate;		// Number of audio samples per second (44100)
	uint16_t blockAlign;		// Number of bytes per sample per channel
	uint16_t bitsPerSample;		// Number of bits per sample
	uint32_t audioSampleBytes;	// Number of bytes to read
	uint32_t audioSampleTables; // Number of wavData tables to read
	
	uint8_t firstDataByteIndex;	// Index of first data block in file
	
	// Metadata section
	uint8_t title[MAXMETADATACHARS];	// Default value: Unknown title
	uint8_t artist[MAXMETADATACHARS];	// Default value: Unknown artist
	uint8_t album[MAXMETADATACHARS];	// Default value: Unknown album
	uint16_t creationYear;
	uint16_t sDuration;
	
}AudioInfo;


/************************************************************************/
/* VARIABLES                                                            */
/************************************************************************/

extern uint16_t audioL;
extern uint16_t audioR;
extern AudioInfo fileData;
extern uint8_t currentVolume;



/************************************************************************/
/* FUNCTIONS	                                                        */
/************************************************************************/

/* audio_setVolume
 *
 * This function sends 2 bytes to N4: AD5300BRMZ to set its analog output.
 * The analog output sets in turn the volume of the N7: TPA6012A4 audio
 * amplifier.
 * 
 * Created 06.11.17 MLN
 * Last modified 13.11.17 MLN
 */
void audio_setVolume (uint8_t);



/* audio_setFileToPlay
 * 
 * Verify the file  and initalise all the data we need to play it.
 * 
 * Created 24.11.17 QVT
 * Last modified 24.11.17 QVT
 */
uint8_t audio_setFileToPlay(uint8_t fileNumber);



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
uint8_t audio_playFile(uint8_t);



/* audio_pauseFile
 *
 * Pauses current .wav file playback
 *
 * Created 20.11.17 MLN
 * Last modified 20.11.17 MLN
 */
void audio_pauseFile(void);

void audio_togglePausePlay(void);

void audio_stop(void);

/* audio_freqStart
 *
 * Starts tc0 to generate a square wave at argument frequency.
 *
 * Created 08.11.17 MLN
 * Last modified 10.11.17 MLN
 */
void audio_freqStart (uint16_t);



/* audio_freqStop
 *
 * Stops previously started tc0.
 *
 * Created 08.11.17 MLN
 * Last modified 10.11.17 MLN
 */
void audio_freqStop (void);



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
void tc1_irq(void);



/* Timer 0 interruption
 *
 * This interruption generates a square wave on audio output.
 * CURRENTLY NOT WORKING, SEE TIMER INITIALIZATION
 *
 * Created 09.11.17 MLN
 * Last modified 13.11.17 MLN
 */
void tc0_irq(void);


#endif /* AUDIO_H */
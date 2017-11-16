/*
 * SDCard.h
 *
 * Created: 08.11.2017 11:21:12
 *  Author: voletqu
 */ 


#ifndef SDCARD_H_
#define SDCARD_H_

#include <asf.h>

/************************************************************************/
/* TYPEDEFS                                                             */
/************************************************************************/

typedef struct file
{
	//uint8_t id;
	char name[25];
	uint32_t sector;
	uint16_t offset;
}File;



/************************************************************************/
/* VARIABLES                                                            */
/************************************************************************/

extern File files[100];



/************************************************************************/
/* PROTOTYPES                                                           */
/************************************************************************/


/* sdcard_init
 *
 * Initialise the spi module
 *
 * Created 16.11.17 QVT
 * Last modified 16.11.17 QVT
 */
void sdcard_init(void);

/* sdcard_mount
 *
 * Execute all the functions to get the SDCard to work.
 *
 * Return false if an error occured.
 * Otherwise return true.
 *
 * Created 16.11.17 QVT
 * Last modified 16.11.17 QVT
 */
bool sdcard_mount(void);

/* sdcard_checkPresence
 *
 * Return true if a SDCard is detected.
 * Otherwise return false.
 *
 * Created 16.11.17 QVT
 * Last modified 16.11.17 QVT
 */
bool sdcard_checkPresence(void);

/* scdcard_setFileToRead
 *
 * Initialise all the required informations
 * for sdcard_getNextSector to work.
 * 
 * Return true if the file exist.
 * Otherwise return false.
 *
 * Created 16.11.17 QVT
 * Last modified 16.11.17 QVT
 */
bool scdcard_setFileToRead(uint8_t fileId);

/* sdcard_getNextSector
 *
 * Copy the next sector data.
 * sdcard_setFileToRead has to be called first 
 * in order for this function to work.
 *
 * Automaticaly change sectors and clusters.
 *
 * Return false if there is no more sector to read.
 * Otherwise return true.
 *
 * Created 16.11.17 QVT
 * Last modified 16.11.17 QVT
 */
bool sdcard_getNextSector(uint8_t *d);

#endif /* SDCARD_H_ */
/*
 * SDCard.c
 *
 * Created: 08.11.2017 11:21:40
 *  Author: voletqu
 */ 

#include "sdcard.h"

/************************************************************************/
/* DEFINES                                                              */
/************************************************************************/

//https://en.wikipedia.org/wiki/Master_boot_record
#define FIRST_PARTITION_OFFSET 446

//https://en.wikipedia.org/wiki/Master_boot_record#PTE
#define SECTOR_OFFSET 8	
#define TYPE_OFFSET 4 


/************************************************************************/
/* TYPEDEF                                                              */
/************************************************************************/

//https://en.wikipedia.org/wiki/Design_of_the_FAT_file_system#BPB
typedef union BiosParameterBlock{
	uint8_t bpb[48];
	struct  
	{
		uint8_t unusedData[11];				//  
		uint8_t bytePerSector[2];			// 
		uint8_t sectorPerCluster[1];		// 
		uint8_t reservedSector[2];			// 
		uint8_t nbrOfFAT[1];				// 
		uint8_t rootSize[2];				// 0 for FAT32
		uint8_t nbrOfSector16bits[2];		// 0 for FAT32
		uint8_t diskType[1];				// 
		uint8_t FATSizeInSectors[2];		// taille de la FAT en secteur si FAT16
		uint8_t nbrOfSectorPerTrack[2];		// 
		uint8_t nbrOfHeads[2];				// 
		uint8_t hiddenSector[4];			// 0 if not partitioned
		uint8_t nbrOfSector32bits[4];		// nombre de secteur si FAT32
		
		//FAT32
		uint8_t FAT32SizeInSectors[4];		// taille de la FAT en secteur si FAT32
		uint8_t diskAttribut[2];			//
		uint8_t FATVersion[2];				//
		uint8_t firstClusterNbr[4];			//
	}components;
}BPB;

typedef struct file
{
	uint8_t id;
	char name[25];
	uint32_t firstSector;
}File;

/************************************************************************/
/* VARIABLES                                                            */
/************************************************************************/

static spi_options_t sdOptions = {
	.reg			=	1,			// NPCS1 pour la carte SD.
	.baudrate		=   1000000,   	// Vitesse de transmission.
	.bits			= 	8,			// 8 bits de long.
	.spck_delay		= 	0,			// Delay entre CS et SPCK.
	.trans_delay	= 	0,			// Delay entre deux transfert.
	.stay_act		=	1,			// ?
	.spi_mode		= 	0,			// ?
	.modfdis		=	1			// ?
};

bool isFAT32 = false;

uint8_t sectorPerClusters;

uint32_t FATSector;
uint32_t rootSector;
uint32_t dataSector;

/************************************************************************/
/* PROTOTYPES                                                           */
/************************************************************************/

bool _initFat();
void _getFilesInfos();



/************************************************************************/
/* FUNCTIONS                                                            */
/************************************************************************/

void sdcard_Init(void){
	gpio_map_t sdGPIO={
		{PIN_NPCS_SD,	FCT_NPCS_SD	  },
		{PIN_SCK_SPI1,	FCT_SCK_SPI1  },
		{PIN_MISO_SPI1, FCT_MISO_SPI1 },
		{PIN_MOSI_SPI1, FCT_MOSI_SPI1 }
	};

	gpio_enable_module(sdGPIO, sizeof (sdGPIO)/ sizeof(sdGPIO[0]));
	

	spi_initMaster((volatile struct avr32_spi_t*) SD_MMC_SPI, &sdOptions);

	spi_selectionMode((volatile struct avr32_spi_t*) SD_MMC_SPI, 0, 0, 0);

	spi_setupChipReg((volatile struct avr32_spi_t*) SD_MMC_SPI, &sdOptions, BOARD_OSC0_HZ);

	spi_enable((volatile struct avr32_spi_t*) SD_MMC_SPI);
}

bool sdcard_Mount(void){
	if(!sdcard_CheckPresence())
		return false;
	if(!sd_mmc_spi_init(sdOptions, BOARD_OSC0_HZ))
		return false;

	return true;
}

bool sdcard_CheckPresence(void){
	return sd_mmc_spi_check_presence(); 
}

bool sdcard_ReadSector(void *ram, uint32_t sector){
	uint8_t *_ram = ram;
	uint16_t  i;
	uint16_t  read_time_out;
	unsigned short data_read;
	uint8_t   r1;

	// wait for MMC not busy
	if (false == sd_mmc_spi_wait_not_busy())
	return false;

	spi_selectChip(SD_MMC_SPI, SD_MMC_SPI_NPCS);    // select SD_MMC_SPI

	// issue command
	if(card_type == SD_CARD_2_SDHC) {
		r1 = sd_mmc_spi_command(MMC_READ_SINGLE_BLOCK, sector);
		} else {
		r1 = sd_mmc_spi_command(MMC_READ_SINGLE_BLOCK, sector<<9);
	}

	// check for valid response
	if (r1 != 0x00)
	{
		spi_unselectChip(SD_MMC_SPI, SD_MMC_SPI_NPCS);  // unselect SD_MMC_SPI
		return false;
	}

	// wait for token (may be a datablock start token OR a data error token !)
	read_time_out = 30000;
	while((r1 = sd_mmc_spi_send_and_read(0xFF)) == 0xFF)
	{
		read_time_out--;
		if (read_time_out == 0)   // TIME-OUT
		{
			spi_unselectChip(SD_MMC_SPI, SD_MMC_SPI_NPCS); // unselect SD_MMC_SPI
			return false;
		}
	}

	// check token
	if (r1 != MMC_STARTBLOCK_READ)
	{
		spi_write(SD_MMC_SPI,0xFF);
		spi_unselectChip(SD_MMC_SPI, SD_MMC_SPI_NPCS);  // unselect SD_MMC_SPI
		return false;
	}

	// store datablock
	for(i=0;i<MMC_SECTOR_SIZE;i++)
	{
		spi_write(SD_MMC_SPI,0xFF);
		spi_read(SD_MMC_SPI,&data_read);
		*_ram++=data_read;
	}
	//gl_ptr_mem += 512;     // Update the memory pointer.

	// load 16-bit CRC (ignored)
	spi_write(SD_MMC_SPI,0xFF);
	spi_write(SD_MMC_SPI,0xFF);

	// continue delivering some clock cycles
	spi_write(SD_MMC_SPI,0xFF);
	spi_write(SD_MMC_SPI,0xFF);

	// release chip select
	spi_unselectChip(SD_MMC_SPI, SD_MMC_SPI_NPCS);  // unselect SD_MMC_SPI

	return true;   // Read done.
}

/*
 *
 *
 */
bool _initFat(){
	uint8_t data[512];
	uint32_t sector = 0;
	
	BPB bpb;
	
	uint8_t nbrOfFAT;
	uint16_t bytesPerSector;
	uint16_t nbrOfReservedSectors;
	uint16_t rootSize;
	uint32_t FATSize;
	
	if(sdcard_ReadSector(&data, 0)){
		
		sector  = data[FIRST_PARTITION_OFFSET + SECTOR_OFFSET + 0];
		sector += data[FIRST_PARTITION_OFFSET + SECTOR_OFFSET + 1] << 8;
		sector += data[FIRST_PARTITION_OFFSET + SECTOR_OFFSET + 2] << 16;
		sector += data[FIRST_PARTITION_OFFSET + SECTOR_OFFSET + 3] << 24;
		
		if(data[FIRST_PARTITION_OFFSET + TYPE_OFFSET] == 0x0B)
			isFAT32 = true;
		else 
			isFAT32 = false;
			
		if(!sdcard_ReadSector(&data, sector))
			return false;
		
		for(int i = 0; i < 48; i++){ //48 = BPB length
			bpb.bpb[i] = data[i];
		}
		
		nbrOfFAT = bpb.components.nbrOfFAT[0];
		bytesPerSector = bpb.components.bytePerSector[0] + (bpb.components.bytePerSector[1] << 8);
		nbrOfReservedSectors = bpb.components.reservedSector[0] + (bpb.components.reservedSector[1] << 8);
		rootSize = bpb.components.rootSize[0] + (bpb.components.rootSize[1] << 8);
		
		if(isFAT32){
			FATSize = bpb.components.FAT32SizeInSectors[0] +
					 (bpb.components.FAT32SizeInSectors[1] << 8) +
					 (bpb.components.FAT32SizeInSectors[2] << 16) +
					 (bpb.components.FAT32SizeInSectors[3] << 24);
		}
		else{
			FATSize = bpb.components.FATSizeInSectors[0] +
					 (bpb.components.FATSizeInSectors[1] << 8);
		}
		
		rootSector = sector + nbrOfReservedSectors + (FATSize * nbrOfFAT);
		FATSector = sector + nbrOfReservedSectors;
		//directory entrees store on 32 bytes
		dataSector = rootSector + ((isFAT32)?(0):(((rootSize * 32) + bytesPerSector - 1) / bytesPerSector));	
	}
	else{
		return false;
	}
	return true;
}

/*
 *
 *
 */
void _getFilesInfos(){
	uint8_t id;
	uint16_t entry;
	uint32_t sector = rootSector;
	uint32_t data[512];
	
	while(1){
		if(entry % 16 == 0){
			if(sdcard_ReadSector(&data, sector))
				sector++;
			else 
				return;
		}
		
		if
	}
}
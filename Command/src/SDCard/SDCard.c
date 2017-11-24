/*
 * SDCard.c
 *
 * Created: 08.11.2017 11:21:40
 *  Author: voletqu
 */ 

#include "sdcard.h"

//#include "GFX/gfx.h"

/************************************************************************/
/* DEFINES                                                              */
/************************************************************************/

//https://en.wikipedia.org/wiki/Master_boot_record
#define FIRST_PARTITION_OFFSET 446

//https://en.wikipedia.org/wiki/Master_boot_record#PTE
#define SECTOR_OFFSET 8	
#define TYPE_OFFSET 4 

#define FILE_END_32	0x0FFFFFF8
#define FILE_END_16	0x0000FFEF


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

typedef union DirectoryEntry{
	uint8_t de[32];
	struct{
		uint8_t shortName[8];
		uint8_t sortExtension[3];
		uint8_t fileAttributes[1];
		uint8_t unusedData[2];
		uint8_t creationTime[2];
		uint8_t creationDate[2];
		uint8_t lastAccessDate[2];
		uint8_t msbCluster[2];
		uint8_t lastModifiedTime[2];
		uint8_t lastModifiedDate[2];
		uint8_t lsbCluster[2];
		uint8_t fileSize[4];
	}components;
}DE;



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

uint8_t sectorPerCluster;

uint32_t FATSector;
uint32_t rootSector;
uint32_t dataSector;

File files[100];
uint8_t nbrOfFiles;

//data is a global variable as we need it in multiple functions
//this way we spare time creating it only once
uint8_t data[512];

uint8_t dummyData[512] = {
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
};

uint32_t cluster;
uint32_t sector;
uint32_t clustersFirstSector;
uint8_t division;
uint32_t sectorsEnd;

volatile bool endOfTransfer = false;


/************************************************************************/
/* PROTOTYPES                                                           */
/************************************************************************/

static bool _initFat();
static void _getFilesInfos();
uint32_t _getFirstCluster(uint8_t fileId);
static bool _readSector(void *ram, uint32_t sector);
bool _readSectorFast(void *ram, uint32_t sector);

void _pdcaInit(void);
__attribute__((__interrupt__)) void pdcaISR(void);

/************************************************************************/
/* FUNCTIONS                                                            */
/************************************************************************/

void sdcard_init(void){
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

bool sdcard_mount(void){
	if(!sdcard_checkPresence())
		return false;
	if(!sd_mmc_spi_init(sdOptions, BOARD_OSC0_HZ))
		return false;
	if(!_initFat())
		return false;
	_getFilesInfos();
	_pdcaInit();
	//
	//gfx_BeginNewTerminal((Vector2){20,200});
	//gfx_AddLineToTerminal(files[0].name, 25, (Color){WHITE},false);
	//gfx_AddLineToTerminal(files[1].name, 25, (Color){WHITE},false);
	//gfx_AddLineToTerminal(files[2].name, 25, (Color){WHITE},false);

	return true;
}

bool sdcard_checkPresence(void){
	return sd_mmc_spi_check_presence(); 
}

bool sdcard_setFileToRead(uint8_t fileId){
	cluster = files[fileId].firstCluster;
	if(cluster == 0)
		return false;
	sector = (cluster - 2) * sectorPerCluster + dataSector;
	clustersFirstSector = sector;
	
	if(isFAT32){
		division = 7;
		sectorsEnd = FILE_END_32;
	}
	else{
		division = 8;
		sectorsEnd = FILE_END_16;
	}
	return true;
}

bool sdcard_getNextSector(uint8_t *d){
	if (sector == sectorsEnd)
		return false;
		
	_readSector(d, sector++);
	
	//next sector and next cluster
	if(sector >= clustersFirstSector + sectorPerCluster){
		_readSector(&data, (cluster >> division) + FATSector);
		if(isFAT32){
			cluster = data[0x01FF & (cluster * 4)]
					+(data[0x01FF & (cluster * 4 + 1)] << 8)
					+(data[0x01FF & (cluster * 4 + 2)] << 16)
					+(data[0x01FF & (cluster * 4 + 3)] << 24);
		}
		else{
			cluster = data[0x01FF & (cluster * 2)]
					+(data[0x01FF & (cluster * 2 + 1)] << 8);
		}
		sector = (cluster - 2) * sectorPerCluster + dataSector;
		clustersFirstSector = sector;
	}
	return true;
}

bool sdcard_getNextSectorFast(uint8_t *d){
	if (sector == sectorsEnd)
	return false;
	
	_readSectorFast(d,sector++);
	
	if(sector >= clustersFirstSector + sectorPerCluster){
		_readSectorFast(&data, (cluster >> division) + FATSector);
		if(isFAT32){
			cluster = data[0x01FF & (cluster * 4)]
			+(data[0x01FF & (cluster * 4 + 1)] << 8)
			+(data[0x01FF & (cluster * 4 + 2)] << 16)
			+(data[0x01FF & (cluster * 4 + 3)] << 24);
		}
		else{
			cluster = data[0x01FF & (cluster * 2)]
			+(data[0x01FF & (cluster * 2 + 1)] << 8);
		}
		sector = (cluster - 2) * sectorPerCluster + dataSector;
		clustersFirstSector = sector;
	}
	return true;
}

/* _readSector
 *
 * function from sd_mmc adapted to our needs
 *
 * Created 16.11.17 QVT
 * Last modified 16.11.17 QVT
 */
static bool _readSector(void *ram, uint32_t sector){
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

bool _readSectorFast(void *ram, uint32_t sector){
	pdca_load_channel( 0, ram, 512);
	pdca_load_channel( 1, dummyData, 512);
	endOfTransfer = false;
	
	if(sd_mmc_spi_read_open_PDCA(sector)){
		pdca_enable_interrupt_transfer_complete(0);
		
		spi_selectChip(SD_MMC_SPI, SD_MMC_SPI_NPCS);
		spi_write(SD_MMC_SPI, 0xFF);
		
		pdca_enable(0);
		pdca_enable(1);
		while(!endOfTransfer);
		return true;
	}else{
		sd_mmc_spi_read_close_PDCA();
		return false;
	}
}

/* _initFat
 *
 * Initialise all the required informations to read the SDCard properly
 *
 * Created 16.11.17 QVT
 * Last modified 16.11.17 QVT
 */
static bool _initFat(){
	uint32_t sector = 0;
	
	BPB bpb;
	
	uint8_t nbrOfFAT;
	uint16_t bytesPerSector;
	uint16_t nbrOfReservedSectors;
	uint16_t rootSize;
	uint32_t FATSize;
	
	if(_readSector(&data, 0)){
		
		sector  = data[FIRST_PARTITION_OFFSET + SECTOR_OFFSET + 0];
		sector += data[FIRST_PARTITION_OFFSET + SECTOR_OFFSET + 1] << 8;
		sector += data[FIRST_PARTITION_OFFSET + SECTOR_OFFSET + 2] << 16;
		sector += data[FIRST_PARTITION_OFFSET + SECTOR_OFFSET + 3] << 24;
		
		if(data[FIRST_PARTITION_OFFSET + TYPE_OFFSET] == 0x0B)
			isFAT32 = true;
		else 
			isFAT32 = false;
			
		if(!_readSector(&data, sector))
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
		
		sectorPerCluster = bpb.components.sectorPerCluster[0];
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

/* _getFilesInfos
 *
 * go through the first 100 files and store 
 * their name, directory sector, and in-sector offset
 *
 * Created 16.11.17 QVT
 * Last modified 16.11.17 QVT
 */
static void _getFilesInfos(){
	uint8_t id = 0;
	uint16_t entry = 0;
	volatile uint32_t sector = rootSector;
	uint32_t relativeEntry = 0;
	nbrOfFiles = 0;
	
	while(entry < 512 && id < 100){
		relativeEntry = (entry % 16) * 32;
		if(entry % 16 == 0){
			if(_readSector(&data, sector))
				sector++;
			else 
				return;
		}
		if((data[relativeEntry + 11] == 0x00 || data[relativeEntry + 11] == 0x01 || data[relativeEntry + 11] == 0x20) && data[relativeEntry] != 0xE5){
			if(data[relativeEntry + 8] == 'W' && data[relativeEntry + 9] == 'A' && data[relativeEntry + 10] == 'V'){
				//files[id].id = id;
				if(files[id].name[0] == 0){
					for(uint8_t  i = 0; i < 11; i++){
						files[id].name[i] = data[relativeEntry + i];
					}
				}
				//files[id].sector = sector;
				files[id].firstCluster = data[relativeEntry + 26]
								+(data[relativeEntry + 27] << 8)
								+(data[relativeEntry + 20] << 16)
								+(data[relativeEntry + 21] << 24);
				id++;
			}
		}
		else if(data[relativeEntry + 11] == 0x0F){
			if(data[relativeEntry] & 0x01 || data[relativeEntry] == 0x41){
				for(uint8_t i = 0; i < 5; i++){
					files[id].name[i] = data[i*2 + relativeEntry + 1];
				}
				for(uint8_t i = 0; i < 7; i++){
					files[id].name[i+5] = data[i*2 + relativeEntry + 14];
				}
				for(uint8_t i = 0; i < 2; i++){
					files[id].name[i+11] = data[i*2 + relativeEntry + 28];
				}
			}
			else if(data[relativeEntry] == 0x02 || data[relativeEntry] == 0x42){
				for(uint8_t i = 0; i < 5; i++){
					files[id].name[i+13] = data[i*2 + relativeEntry + 1];
				}
				for(uint8_t i = 0; i < 5; i++){ // 5 instead of 7 as we only store the first 25 characters
					files[id].name[i+18] = data[i*2 + relativeEntry + 14];
				}
			}
		}
		entry++;
	}
	nbrOfFiles = id;
}

void _pdcaInit(void)
{

	// Ce cannal PDCA est utilisé pour la réception des données depuis le SPI.
	pdca_channel_options_t pdca_options_SPI_RX = { // pdca channel options
		.addr = (uint8_t *)&data,					// adresse dans la Ram.
		.size = 512,                                // transfer counter: here the size of the string
		.r_addr = NULL,                             // next memory address after 1st transfer complete
		.r_size = 0,                                // next transfer counter not used here
		.pid = 8,          // select peripheral ID - data are on reception from SPI1 RX line
		.transfer_size = PDCA_TRANSFER_SIZE_BYTE    // select size of the transfer: 8,16,32 bits
	};

	// Ce canal est utilisé pour envoyer un dummy
	pdca_channel_options_t pdca_options_SPI_TX = { // pdca channel options
		.addr = (uint8_t *)&dummyData,             // memory address. We take here the address of the string dummy_data.
		.size = 512,                                // transfer counter: here the size of the string
		.r_addr = NULL,                             // next memory address after 1st transfer complete
		.r_size = 0,                                // next transfer counter not used here
		.pid = 16,          // select peripheral ID - data are on emission from SPI1 TX line
		.transfer_size = PDCA_TRANSFER_SIZE_BYTE    // select size of the transfer: 8,16,32 bits
	};

	// Init PDCA transmission channel
	pdca_init_channel(1, &pdca_options_SPI_TX);
	// Init PDCA Reception channel
	pdca_init_channel(0, &pdca_options_SPI_RX);
	//! \brief Enable pdca transfer interrupt when completed
	INTC_register_interrupt(&pdcaISR, AVR32_PDCA_IRQ_0, AVR32_INTC_INT1);  // pdca_channel_spi0_RX = 0
}

__attribute__((__interrupt__)) void pdcaISR(void)
{
	// Disable all interrupts.
	//Disable_global_interrupt();
	// Disable interrupt channel.
	pdca_disable_interrupt_transfer_complete(0);
	sd_mmc_spi_read_close_PDCA();//unselects the SD/MMC memory.
	// Disable unnecessary channel
	pdca_disable(1);
	pdca_disable(0);
	// Enable all interrupts.
	//Enable_global_interrupt();
	endOfTransfer = true;
}
/*
 * SDCard.c
 *
 * Created: 08.11.2017 11:21:40
 *  Author: voletqu
 */ 

#include "sdcard.h"

// Master Boot Entry
typedef union {
	struct {
		uint8_t MBE[16] ;			//Partition Entry.
	} tableau;
	struct {
		uint8_t amorcage;			//Partition amorçable = 80h
		uint8_t debut[3];
		uint8_t type;				//Type de partition
		uint8_t fin[3];
		uint8_t start[4];			//Début de la partition (secteurs cachés)
		uint8_t taille[4];			//Taille de la partition en secteurs
	} structure;
} MBE_t;

// Boot Sector Entry
typedef union {
	struct {
		uint8_t	SBE[61];			// Boot Sector Entry.
	} tableau;
	struct	{
		uint8_t	jump[3];			// Instruction de saut.
		uint8_t OEM[8];				// OEM Nom (MSDOS 5.0)
		uint8_t sec_octets[2];		// Nombre d'octets par secteur.
		uint8_t sec_cluster[1];		// Nombre de secteurs par cluster.
		uint8_t	sec_reserved[2];	// Nombre de secteurs réservés.
		uint8_t fat_copie[1];		// Nombre de copies de fat.
		uint8_t	rep_entry[2];		// Nombre d'entrées possibles dans répertoire racine.
		uint8_t sec_number[2];		// Nombre de secteur pour des capacité < 32MBytes
		uint8_t type[1];			// Type de support.
		uint8_t	fat_size[2];		// Taille d'une FAT en secteurs.
		uint8_t	sec_track[2];		// Nombre de secteurs par piste.
		uint8_t	head[2];			// Nombre de tête.
		uint8_t sec_hide[4];		// Nombre de secteurs cachés.
		uint8_t sec_size[4];		// Nombre de secteurs de la partition.
		uint8_t	disk[1];			// disque physique.
		uint8_t active_head[1];		// Tête active.
		uint8_t boot_sig[1];		// Boot signature.
		uint8_t	serial_number[4];	// Numéro de série.
		uint8_t volume_name[11];	// Nom du volume.
	} structure;
} SBE_t;


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

volatile bool FAT;
static MBE_t master_boot;
static SBE_t sector_boot;
volatile RDE_t file_name;
static uint32_t	sectors;
static uint32_t	secteur_data;	//premier secteur des données.
static uint32_t	secteur_RDE;	//premier secteur de la Root Directory.
static uint32_t	secteur_FAT;
static uint8_t data_mem[513]; 
volatile file_menu_t file_menu[100];	

static bool _setupFAT(void);
static uint8_t _filesName(void);

bool sdcard_ReadSector(void *ram, uint32_t sector);

void sdcard_Init(void){
	gpio_map_t sdGPIO={
		{PIN_NPCS_SD,	FCT_NPCS_SD	  },
		{PIN_SCK_SPI1,	FCT_SCK_SPI1  },
		{PIN_MISO_SPI1, FCT_MISO_SPI1 },
		{PIN_MOSI_SPI1, FCT_MOSI_SPI1 }
	};

	gpio_enable_module(sdGPIO, sizeof (sdGPIO)/ sizeof(sdGPIO[0]));
	

	spi_initMaster((volatile struct avr32_spi_t*) SD_MMC_SPI,&sdOptions);

	spi_selectionMode((volatile struct avr32_spi_t*) SD_MMC_SPI, 0, 0, 0);

	spi_setupChipReg((volatile struct avr32_spi_t*) SD_MMC_SPI, &sdOptions,BOARD_OSC0_HZ);

	spi_enable((volatile struct avr32_spi_t*) SD_MMC_SPI);
}

bool sdcard_Mount(void){
	if(!sdcard_CheckPresence())
		return false;
	if(!sd_mmc_spi_init(sdOptions, BOARD_OSC0_HZ))
		return false;
	if(!_setupFAT())
		return false;
	_filesName();

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

bool Root_directory(uint8_t file_numero){
	uint32_t 	temp;
	uint16_t 	entry;
	uint32_t 	i;

	// Une entrée = 32 octets (512/32)=16 entrées possibles dans Root.
	//file_numero++;
	entry = -1;
	sectors = secteur_RDE;
	if (file_numero < 0) return false;
	
	/* Principe tantque file_numero et différent de 0 on reste dans la boucle do
	   while, si une entrée de la root directory est un fichier effacer ou une 
	   entrée longue la variable file_numero n'est pas décrémentée */
	do
	{
	  entry++;
	  if ((entry % 16) == 0)
	  {
		if (sdcard_ReadSector((uint8_t *) &data_mem, sectors) == true) sectors++;
		else return false;
	  }
	  
	  //si le premier caractère = 0xE5 c'est un fichier effacé ou 0x0F entrée longue	
	  if ((data_mem[11+((entry%16)*32)] != 0x0F) && (data_mem[0+((entry%16)*32)] != 0xE5))
	  {
			if (file_numero == 0)
			{
				for (i = 0; i < 32; i++)
				file_name.tableau.RDE[i] = data_mem[((entry%16)*32)+i];
				return true;
			}
			else file_numero--;  
	  }
	  
	}while (entry < 1024); //512 Entrées possible dans la Root Directory.
	
	if (entry >= 512) return false;
	else return true;
}

static bool _setupFAT(void){
	uint8_t		i;
	uint32_t	j;
		
	uint32_t	RootdirSector;
	uint32_t	FisrtDataSectors;
	uint32_t	Adresse_BS;
	uint32_t	Adresse_RD;
		
	uint32_t	BPB_ResvdSecCnt;
	uint32_t	BPB_BytsPerSec;
	uint32_t	BPB_RootEntCnt;
	uint32_t	BPB_NumFATs;
	uint32_t	FATsize;
		
	FAT = 0;
		
	//if (sd_spi_quick_read_sector_to_ram((uint8_t *) &data_mem, 0) == true)
	if (sdcard_ReadSector((uint8_t *) &data_mem, 0) == true)
	{
		for (i = 0; i < 16; i++)
		{
			master_boot.tableau.MBE[i] = data_mem[446 + i];
		}
		// Décalage pour la lecture du Sector Boot
		sectors = master_boot.structure.start[0];
		sectors += master_boot.structure.start[1] << 8;
		sectors += master_boot.structure.start[2] << 16;
		sectors += master_boot.structure.start[3] << 24;
			
		//bizarrement en FAT32 l'adresse start ne correspond pas aux secteurs cachés
		// mais à l'adresse des premiers bytes du secteur boot ??
		if (master_boot.structure.type == 0x0b)
		{
			//sectors /= 512;
			FAT = 1;
		}
			
		if (sdcard_ReadSector((uint8_t *) &data_mem, sectors) == true)
		{
			for (i = 0; i < 61; i++)
			{
				sector_boot.tableau.SBE[i] = data_mem[i];
			}
		}
			
		/* Nouveau calcul d'adresse de Root directory */
		if (FAT == 0)
		{
			Adresse_BS = sectors;
			BPB_RootEntCnt = sector_boot.structure.rep_entry[0] + (sector_boot.structure.rep_entry[1] << 8);
			BPB_BytsPerSec = sector_boot.structure.sec_octets[0] + (sector_boot.structure.sec_octets[1] << 8);
			BPB_ResvdSecCnt = sector_boot.structure.sec_reserved[0] + (sector_boot.structure.sec_reserved[1] << 8);
			BPB_NumFATs = sector_boot.structure.fat_copie[0];
			FATsize = sector_boot.structure.fat_size[0] + (sector_boot.structure.fat_size[1] << 8);
				
			RootdirSector = ((BPB_RootEntCnt * 32) + (BPB_BytsPerSec - 1)) / BPB_BytsPerSec;			// Nombre de secteur de la Rootdirectory
			FisrtDataSectors = Adresse_BS + BPB_ResvdSecCnt + (BPB_NumFATs * FATsize) + RootdirSector;  // Premier secteur de données
			secteur_RDE = Adresse_BS + BPB_ResvdSecCnt + (BPB_NumFATs * FATsize);
			secteur_FAT = Adresse_BS + BPB_ResvdSecCnt;
			secteur_data = FisrtDataSectors;
		}
		else  // L'adresse de la Root directory en FAT32
		{
			Adresse_BS = sectors;
			BPB_BytsPerSec = sector_boot.structure.sec_octets[0] + (sector_boot.structure.sec_octets[1] << 8);
			BPB_ResvdSecCnt = sector_boot.tableau.SBE[14] + (sector_boot.tableau.SBE[15] << 8);
			FATsize = sector_boot.tableau.SBE[36] + (sector_boot.tableau.SBE[37] << 8) +
			(sector_boot.tableau.SBE[38] << 16) + (sector_boot.tableau.SBE[39] << 24);
			BPB_NumFATs = sector_boot.tableau.SBE[16];
				
			//RootdirSector = sector_boot.structure.sec_cluster[0];			// 1 cluster pour la Root Directory
			RootdirSector = 0x00;
			FisrtDataSectors = Adresse_BS + BPB_ResvdSecCnt + (BPB_NumFATs * FATsize) + RootdirSector;  // Premier secteur de données
			secteur_RDE = Adresse_BS + (BPB_ResvdSecCnt + (FATsize * BPB_NumFATs));
			secteur_FAT = Adresse_BS + BPB_ResvdSecCnt;
			secteur_data = FisrtDataSectors;
		}
		return true;
	}
	else return false;
}

static uint8_t _filesName(void)
{
	uint32_t 	temp;
	uint16_t 	entry;
	uint8_t 	i;
	uint8_t		file_numero = 0;

	// Une entrée = 32 octets (512/32)=16 entrées possibles dans Root.
	entry = -1;
	sectors = secteur_RDE;
	
	/* Efface les titres enregistrés */
	for(file_numero = 0; file_numero < 100; file_numero++)
	{
		for (i = 0; i < 25; i++) file_menu[file_numero].name[i] = 0x20;
		file_menu[file_numero].name[24] = 0x00;
		file_menu[file_numero].sector[3] = 0x00;
		file_menu[file_numero].sector[2] = 0x00;
		file_menu[file_numero].sector[1] = 0x00;
		file_menu[file_numero].sector[0] = 0x00;
				
		file_menu[file_numero].entry[0] = 0x00;
		file_menu[file_numero].entry[1] = 0x00;
		file_menu[file_numero].num = 0;
	}
	file_numero = 0;
	
	/* Lecture de la carte SD et stock les noms des fichiers ainsi que le secteur (sector) 
	   et la position dans le secteur (entry) */
	
	do
	{
	  entry++;
	  if ((entry % 16) == 0)
	  {
		if (sdcard_ReadSector((uint8_t *) &data_mem, sectors) == true) sectors++;
		else return false;
	  }
	  
	  //si le premier caractère = 0xE5 c'est un fichier effacé ou 0x0F entrée longue
	  temp = ((entry % 16) * 32);
	  if ((data_mem[11+temp] != 0x0F) && (data_mem[temp] != 0xE5))
	  {
		  // Récupère uniquement les fichiers de type WAVE
		  if ((data_mem[8+temp] == 'W') && (data_mem[9+temp] == 'A') && (data_mem[10+temp] == 'V'))
		  {	
			  // Si il s'agit d'un nom de fichier court récupère la première partie du nom
			  if (file_menu[file_numero].name[0] == 0)
			  {	 
				 for (i = 0; i < 8; i++) file_menu[file_numero].name[i] = data_mem[i+temp];
				 for (i = 0; i < 3; i++) file_menu[file_numero].name[i + 8] = data_mem[i+temp+8];
			  }
			  file_menu[file_numero].sector[3] = (uint8_t) sectors;
			  file_menu[file_numero].sector[2] = (uint8_t) (sectors >> 8);
			  file_menu[file_numero].sector[1] = (uint8_t) (sectors >> 16);
			  file_menu[file_numero].sector[0] = (uint8_t) (sectors >> 24);
				
			  file_menu[file_numero].entry[0] = (uint8_t) entry;
			  file_menu[file_numero].entry[1] = (uint8_t) (entry >> 8);
			  file_menu[file_numero].num = file_numero;
			  file_numero++;
		  }
	  }
	  
	  // Première partie d'un nom de fichier long
	  else if ((data_mem[11+temp] == 0x0F) && ((data_mem[temp] == 0x01) || (data_mem[temp] == 0x41)))
	  {
		  for (i = 0; i < 5; i++) file_menu[file_numero].name[i] = data_mem[(i*2)+temp+1];
		  for (i = 0; i < 7; i++) file_menu[file_numero].name[i + 5] = data_mem[(i*2)+temp+14];
		  for (i = 0; i < 2; i++) file_menu[file_numero].name[i + 11] = data_mem[(i*2)+temp+28];
	  }  
	  
	  // Deuxième partie d'un nom de fichier long
	  else if ((data_mem[11+temp] == 0x0F) && ((data_mem[temp] == 0x02) || (data_mem[temp] == 0x42)))
	  {
		 for (i = 0; i < 5; i++) file_menu[file_numero].name[i + 13] = data_mem[(i*2)+temp+1];
		 for (i = 0; i < 5; i++) file_menu[file_numero].name[i + 18] = data_mem[(i*2)+temp+14];
		 //for (i = 0; i < 2; i++) file_menu[file_numero].name[i + 24] = data_mem[(i*2)+temp+28];
	  }
	}while ((entry < 512) && (file_numero < 100)); //512 Entrées possible dans la Root Directory.

return file_numero;
}

/*
 * SDCard.h
 *
 * Created: 08.11.2017 11:21:12
 *  Author: voletqu
 */ 


#ifndef SDCARD_H_
#define SDCARD_H_

#include <asf.h>

void sdcard_Init(void);

bool sdcard_Mount(void);

bool sdcard_CheckPresence(void);

bool sdcard_ReadSector(void *ram, uint32_t sector);

//funky fanky code below

bool Root_directory(uint8_t file_numero);

typedef struct file_menu_t
{
	uint8_t		num;			//Numéro du fichier
	uint8_t		sector[4];		//secteur de la SD Card
	uint8_t		entry[2];		//position dans la Root directory
	uint8_t		name[25];		//nom du fichier (String)
} file_menu_t;

extern volatile file_menu_t file_menu[100];

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

extern SBE_t sector_boot;
// Root Directory Entry
typedef union {
	struct {
		uint8_t	RDE[36];			// Root directory Entrée.
	} tableau;
	struct {
		uint8_t	nom[8];				// Nom du fichier 8 caractères.
		uint8_t	extension[3];		// Extension du fichier 3 caractères.
		uint8_t attribut[1];		// Attributs 0Fh indique un nom de fichier long.
		uint8_t	res1[2];
		uint8_t	heure_mod[2];		// L'heure de modification.
		uint8_t	date_mod[2];		// Date de la modification.
		uint8_t date_end[2];		// Date de la dernière modification.
		uint8_t	res2[2];
		uint8_t	heure_creation[2];	// Heure de la création du fichier.
		uint8_t	date_creation[2];	// Date de création.
		uint8_t	First_cluster[2];	// Premier cluster (secteur)
		uint8_t	taille[4];			// taille du fichier en octets.
		uint32_t sector[1];			// Numéro du premier secteur du fichier.
	} structure;
} RDE_t;

extern uint32_t	secteur_data;
extern volatile RDE_t file_name;
extern volatile bool FAT;

#endif /* SDCARD_H_ */
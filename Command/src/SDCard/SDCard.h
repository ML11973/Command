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

bool Root_directory(uint8_t file_numero);

typedef struct file_menu_t
{
	uint8_t		num;			//Numéro du fichier
	uint8_t		sector[4];		//secteur de la SD Card
	uint8_t		entry[2];		//position dans la Root directory
	uint8_t		name[25];		//nom du fichier (String)
} file_menu_t;

extern volatile file_menu_t file_menu[100];

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

extern volatile RDE_t file_name;
extern volatile bool FAT;

#endif /* SDCARD_H_ */
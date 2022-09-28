/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:add_device.c	1.3"

/* #define	DEBUG		1	/* */

/*
		add_device(handle)

	Pass this routine a handle and it will add it to our
	internal table of specs.

	Returns -1 on failure.
*/
#include	<stdio.h>

#include	"MS-DOS.h"

add_device(handle)
int	handle;
{
	int	index;
	FILE	*output;
	char	identifier[30];

#ifdef DEBUG
	output = stderr;
	(void) strcpy(identifier, "add_device(): DEBUG - ");
	we_are_dosinfo = 0;
#endif

	if (we_are_dosinfo) {
		output = stdout;
		identifier[0] = '\0';
	}

	/*
		Find empty slot in our list.
	*/
	for (index = 0; index < MAX_DEVICES &&  TABLE.handle != 0; index++);

	/*
		If no slot available - complain
	*/
	if (index == MAX_DEVICES) {
		(void) fprintf(stderr, "add_device(): Device Table is full\n");
		return(-1);
	}

	/*
		We found a slot - fill it.

		First we do a short read of the boot sector.
		Once we do this we can re-read it for total size etc.
	*/
	TABLE.handle = handle;
	TABLE.bytes_per_sector = 30;

	/* Read partial boot sector */
	if (read_sector(handle, 0) == -1)
		return(-1);

	if (sector_buffer[0] != 0xE9 && sector_buffer[0] != 0xEB) {
		(void) fprintf(stderr, "Non MS-DOS disk\n");
		return(-1);
	}

	/*
		Get all pertinent info from boot sector
	*/
	TABLE.bytes_per_sector = (sector_buffer[LOW_SECTOR_SIZE] + (256 * sector_buffer[HI_SECTOR_SIZE]));

	TABLE.sectors_per_cluster = sector_buffer[SECTORS_PER_CLUSTER];

	TABLE.reserved_sectors = (sector_buffer[LOW_RESERVED_SECTORS] + (256 * sector_buffer[HI_RESERVED_SECTORS]));

	TABLE.number_of_fats = sector_buffer[NUMBER_OF_FATS];

	TABLE.root_dir_ent = sector_buffer[LOW_ROOT_DIR_ENT] + (256 * sector_buffer[HI_ROOT_DIR_ENT]);

	TABLE.total_sectors = sector_buffer[LOW_TOTAL_SECTORS] + (256 * sector_buffer[HI_TOTAL_SECTORS]);

	/*
		We must support 2 FAT entry sizes 16 and 12 bit
	*/
	if (4095 > TOTAL_CLUSTERS)
		TABLE.fat16 = 0;
	else
		TABLE.fat16 = 1;

	TABLE.media_descriptor = sector_buffer[MEDIA_DESCRIPTOR];
	TABLE.sectors_per_fat = sector_buffer[LOW_SECTORS_PER_FAT] + (256 * sector_buffer[HI_SECTORS_PER_FAT]);

	TABLE.root_base_sector = TABLE.reserved_sectors + (TABLE.number_of_fats * TABLE.sectors_per_fat);

	TABLE.sectors_in_root = (TABLE.root_dir_ent * BYTES_PER_DIR) / TABLE.bytes_per_sector;

#ifndef DEBUG
	/*
		If we are not in DEBUG mode, and we are not dosinfo,
		don't let the remainder be executed - surely will dump
		core as not valid value for 'output' has been assigned
	*/
	if (we_are_dosinfo == 0)
		return(0);
#endif

	/*
		Display data if requested 
	*/
	(void) fprintf(output, "%sBytes per sector = %d\n", identifier, TABLE.bytes_per_sector);
	(void) fprintf(output, "%sSectors per cluster = %d\n", identifier, TABLE.sectors_per_cluster);
	(void) fprintf(output, "%sReserved sectors = %d\n", identifier, TABLE.reserved_sectors);
	(void) fprintf(output, "%sNumber of FATs = %d\n", identifier, TABLE.number_of_fats);
	(void) fprintf(output, "%sRoot directory entries = %d\n", identifier, TABLE.root_dir_ent);
	(void) fprintf(output, "%sTotal sectors on volume = %d\n", identifier, TABLE.total_sectors);
	(void) fprintf(output, "%sSectors per FAT = %d\n", identifier, TABLE.sectors_per_fat);
	(void) fprintf(output, "%sfat16 = %d\n", identifier, TABLE.fat16);
	(void) fprintf(output, "%sMedia Descriptor = %x\n", identifier, TABLE.media_descriptor);
	(void) fprintf(output, "%sBase sector of root directory = %ld\n", identifier, TABLE.root_base_sector);
	(void) fprintf(output, "%sSectors in root directory = %ld\n", identifier, TABLE.sectors_in_root);

	return(0);
}

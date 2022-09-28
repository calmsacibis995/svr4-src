/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:open_device.c	1.3"

/* #define		DEBUG		1	/* */

/*
		open_device(device, mode)

	Open a specified device in the specified mode.
	Return handle on success, -1 on failure.
*/

#include	"MS-DOS.h"

#include	<stdio.h>

#include	<fcntl.h>

open_device(dosfilename, mode)
char	*dosfilename;
int	mode;
{
	int	handle;
	int	index;
	long	j;
	int	k;

#ifdef DEBUG
	(void) fprintf(stderr, "open_device(): DEBUG - dosfilename: \"%s\" mode: %d\n", dosfilename, mode);
#endif

	/*
		Load up hardware assignments
	*/
	if (assignments_loaded == 0) {
		if (get_assignments() == -1) {
			(void) fprintf(stderr, "open_device(): Error - Failed to process assignments file - Terminating\n");
			exit(1);
		}

		assignments_loaded++;
	}

	if (parse_name(dosfilename) == -1)
		return(-1);

	if ((index = lookup_drive(drive)) == -1) {
		(void) fprintf(stderr, "open_device(): Error - Drive '%c' not found in assignments file\n", drive);
		return(-1);
	}

	/*
		Open the requested device, in the specified mode.
	*/
#ifdef DEBUG
	(void) fprintf(stderr, "open_device(): DEBUG - Attempting to open device \"%s\"\n", device_pathname);
#endif
	if ((handle = open(device_pathname, mode | O_EXCL)) == -1)  {
		(void) fprintf(stderr, "open_device(): Error - open(\"%s\", %d) failed\n", device_pathname, mode);
		perror("	Reason");
		return(-1);
	} 

	/*
		If open did not fail, then add this device to our table
	*/
	if ((index = add_device(handle)) == -1)
		return(-1);

	/*
		Load our FAT into memory
	*/
#ifdef DEBUG
	(void) fprintf(stderr, "open_device(): DEBUG - Attempting to malloc %ld bytes (%ld * %ld)\n", TABLE.bytes_per_sector * TABLE.sectors_per_fat, TABLE.bytes_per_sector, TABLE.sectors_per_fat);
#endif

	/*
		malloc our working copy of the FAT
	*/
	if ((TABLE.our_fat = malloc((unsigned) (TABLE.bytes_per_sector * TABLE.sectors_per_fat))) == NULL) {
		(void) fprintf(stderr, "open_device(): Error - Failed to malloc FAT space\n");
		exit(1);
	}

#ifdef DEBUG
	(void) fprintf(stderr, "open_device(): DEBUG - malloc of %ld bytes is successful\n", TABLE.bytes_per_sector * TABLE.sectors_per_fat);
#endif
	for (j = TABLE.reserved_sectors; j < TABLE.sectors_per_fat + TABLE.reserved_sectors; j++) {
		if (read_sector(handle, j) == -1)
			exit(1);

		for (k = 0; k < TABLE.bytes_per_sector; k++) {
			*(TABLE.our_fat + k + ((j - TABLE.reserved_sectors) * TABLE.bytes_per_sector)) = sector_buffer[k];
		}
	}

#ifdef DEBUG
	(void) fprintf(stderr, "open_device(): DEBUG - FAT loaded\n");
#endif
	return(handle);
}

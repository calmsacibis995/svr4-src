/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:read_sector.c	1.3"

/* #define		DEBUG		1 	/* */

/*
		read_sector(device, sector)

	Pass this routine a handle and a sector number and it 
	will read that sector from a disk.

	On success returns the number of bytes read.
	Returns -1 on failure.
*/

#include	<stdio.h>

#include	"MS-DOS.h"

int
read_sector(device_handle, sector)
int	device_handle;
long	sector;
{
	int	bytes_read;
	int	index;
	long	offset;

#ifdef DEBUG
	(void) fprintf(stderr, "read_sector(): DEBUG - Reading sector %ld\n", sector);
#endif
	/*
		Locate this handle in our device table
	*/
	if ((index = lookup_device(device_handle)) == -1) {
		(void) fprintf(stderr, "read_sector(): Error - Handle %d not found in device table\n", device_handle);
		return(-1);
	}

	/*
		Calculate the actual displacement into the media
	*/
	offset = TABLE.bytes_per_sector * sector;

	/*
		Seek to the appropriate address for this sector
	*/
	if (lseek(device_handle, offset, 0) == -1) {
		(void) fprintf(stderr, "read_sector(): Failed to lseek to offset %ld\n", offset);
		perror("	Reason");
		return(-1);
	}

	/*
		Do the actual read. We should never have short blocks.
	*/
	if ((bytes_read = read(device_handle, (char *) sector_buffer, TABLE.bytes_per_sector)) != TABLE.bytes_per_sector) {
		(void) fprintf(stderr, "read_sector(): Read error got %d vs %d\n", bytes_read, TABLE.bytes_per_sector);
		return(-1);
	}

#ifdef DEBUG
	(void) fprintf(stderr, "read_sector(): DEBUG - Read of sector %ld complete - Location: %ld - No errors\n", sector, offset);
#endif

	last_sector_read = sector;

	return(bytes_read);
}

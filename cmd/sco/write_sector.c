/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:write_sector.c	1.3"

/* #define		DEBUG		1	/* */

/* #define		PRECIOUS	1	/* */

/*
		write_sector(device, sector)

	Pass this routine a handle and a sector number and it 
	will write that sector to a disk.

	On success returns the number of bytes written.
	Returns -1 on failure.

	Defining PRECIOUS dis-allows actual updating of the disk
	(useful when debugging).
*/

#include	<stdio.h>

#include	"MS-DOS.h"

int
write_sector(device_handle, sector)
int	device_handle;
long	sector;
{
	int	bytes_written;
	int	index;
	long	offset;

#ifdef DEBUG
	(void) fprintf(stderr, "write_sector(): DEBUG - Writing sector %ld\n", sector);
#endif
	/*
		Locate this handle in our device table
	*/
	if ((index = lookup_device(device_handle)) == -1) {
		(void) fprintf(stderr, "write_sector(): Error - Handle %d not found in device table\n", device_handle);
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
		(void) fprintf(stderr, "write_sector(): Failed to lseek to offset %ld\n", offset);
		return(-1);
	}

	/*
		Do the actual write. We should never have short blocks.
	*/
#ifdef PRECIOUS
	bytes_written = TABLE.bytes_per_sector;
#else
	if ((bytes_written = write(device_handle, (char *) sector_buffer, TABLE.bytes_per_sector)) != TABLE.bytes_per_sector) {
		(void) fprintf(stderr, "write_sector(): write error, Is the disk full?\n");
		return(-1);
	}
#endif

#ifdef DEBUG
	(void) fprintf(stderr, "write_sector(): DEBUG - Wrote sector %ld - Location: %ld\n", sector, offset);
#endif
	return(bytes_written);
}

/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:write_fat.c	1.3"

#include	"MS-DOS.h"

#include	<stdio.h>

write_fat(handle)
int	handle;
{
	int	index;
	long	j;
	int	k;

	/*
		Locate this handle in our device table
	*/
	if ((index = lookup_device(handle)) == -1) {
		(void) fprintf(stderr, "write_fat(): Error - Handle %d not found in device table\n", handle);
		return(-1);
	}

	for (j = TABLE.reserved_sectors; j < TABLE.sectors_per_fat + TABLE.reserved_sectors; j++) {
		for (k = 0; k < TABLE.bytes_per_sector; k++)
			sector_buffer[k] = *(TABLE.our_fat + k + ((j - TABLE.reserved_sectors) * TABLE.bytes_per_sector));

		if (write_sector(handle, j) == -1)
			return(-1);

		if (write_sector(handle, j + TABLE.sectors_per_fat) == -1)
			return(-1);
	}

	return(0);
}

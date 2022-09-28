/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:ud_copy.c	1.3"

/* #define		DEBUG		1	/* */

/*
	ud_copy(handle, displacement, conversion, output, starting_cluster, size)

	Pass this routine the displacement within buffer
	of a directory entry for an MS-DOS file, and it will open 
	the file and write it to output_fptr. Return -1 on error.

	Note: Here we convert, if requested, \n to \r\n.
*/

#include	"MS-DOS.h"

#include	<stdio.h>

#define CHECK_SECTOR()	\
	if (sector_index == TABLE.bytes_per_sector) { \
		if (write_sector(output, sector) == -1) \
			return(-1); \
		\
		total_written = total_written + TABLE.bytes_per_sector; \
		sector_index = 0; \
		\
		if ((size -= TABLE.bytes_per_sector) > 0 && ((++sector - (TABLE.root_base_sector + TABLE.sectors_in_root)) % TABLE.sectors_per_cluster == 0)) { \
			if ((n_cluster = alloc_cluster(output)) == -1) \
				return(-1); \
			\
			if (chain_cluster(output, n_cluster, SECT_2_CLUS(sector - 1)) == -1) \
				return(-1); \
			\
			sector = CLUS_2_SECT(n_cluster); \
		} \
	} 

long
ud_copy(handle, conversion, output, start, size)
int	handle;
int	conversion;
int	output;
long	start;
long	size;
{
	char	buffer[MAX_SECTOR_SIZE];
	int	bytes_read;
	int	index;
	long	n_cluster;
	long	sector;
	long	sector_index;
	long	total_written = 0;
	int	work_int;

#ifdef DEBUG
	(void) fprintf(stderr, "ud_copy(): DEBUG - handle: %d conversion: %d output: %d start: %ld\n", handle, conversion, output, start);
#endif

	/*
		Locate this handle in our device table
	*/
	if ((index = lookup_device(output)) == -1) {
		(void) fprintf(stderr, "ud_copy(): Error - Handle %d not found in device table\n", output);
		return(-1);
	}

	sector = CLUS_2_SECT(start);
	sector_index = 0;

	while ((bytes_read = read(handle, buffer, MAX_SECTOR_SIZE)) > 0) {
#ifdef DEBUG
		(void) fprintf(stderr, "ud_copy(): DEBUG - Read an input record of %d bytes\n", bytes_read);
#endif
		for (work_int = 0; work_int < bytes_read; work_int++) {
			if (conversion == 1 && buffer[work_int] == '\n') {
				sector_buffer[sector_index++] = '\r';

				CHECK_SECTOR();
			}

			sector_buffer[sector_index++] = buffer[work_int];

			CHECK_SECTOR();
		}
	}

	if (sector_index) {
		if (write_sector(output, sector) == -1)
			return(-1);

		total_written += sector_index;
	}

#ifdef DEBUG
	(void) fprintf(stderr, "ud_copy(): DEBUG - Total written: %ld\n", total_written);
#endif

	return(total_written);
}


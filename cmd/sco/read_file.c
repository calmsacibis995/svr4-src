/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:read_file.c	1.3"

/* #define		DEBUG		1	/* */

/*
		read_file(handle, displacement, conversion)

	Pass this routine the displacement within sector_buffer
	of a directory entry for an MS-DOS file, and it will open 
	the file and write it to stdout. Return -1 on error.
*/

#include	"MS-DOS.h"

#include	<stdio.h>

read_file(handle, i, conversion, output)
int	handle;
int	i;
int	conversion;
FILE	*output;
{
	long	bytes_to_read = dos_fil_size(i);
	long	cluster;
	int	index;
	int	last_was_cr;
	long	sector;
	int	sectors_left;

	if ((index = lookup_device(handle)) == -1) {
		(void) fprintf(stderr, "read_file(): Error - Handle %d not found in device table\n", handle);
		return(-1);
	}

	/*
		If we are doing a directory, then set bytes_to_read
		to the cluster size.
	*/
	if (sector_buffer[FILE_ATTRIBUTE + i] == SUB_DIRECTORY)
		bytes_to_read = TABLE.bytes_per_sector * TABLE.sectors_per_cluster;

	/*
		Determine starting cluster number. Remember that data
		always starts in the SECOND cluster per DOS Tech. 
		Manual pp. 4-13. Least significant byte is first.
	*/
	cluster = GET_CLUS(i);

#ifdef DEBUG
	(void) fprintf(stderr, "read_file(): DEBUG - starting cluster %ld Displacement: %d\n", cluster, i);
#endif

	/*
		System uses the following sectors:

			boot sectors
			    +
			fat sectors
			    +
			root directory sectors
	*/
#ifdef DEBUG
	(void) fprintf(stderr, "read_file(): DEBUG - System sectors = %d\n", TABLE.root_base_sector + TABLE.sectors_in_root);
#endif

	/*
		Calculate the absolute sector number for this cluster.
	*/
	last_was_cr = 0;
	
	do {
		int	work_int;

		sector = CLUS_2_SECT(cluster);

		for (sectors_left = TABLE.sectors_per_cluster; sectors_left; sectors_left--, sector++) {
			if (read_sector(handle, sector) == -1) {
#ifdef DEBUG
				(void) fprintf(stderr, "read_file(): DEBUG - read_sector(%d, %ld) failed\n", handle, sector);
#endif
				return(-1);
			}
	
			/*
				Display data from this sector
			*/
			for (work_int = 0; bytes_to_read > 0 && work_int < TABLE.bytes_per_sector; work_int++) {
				bytes_to_read--;
	
				if (conversion == 1 && sector_buffer[work_int] == '\r') {
					last_was_cr = 1;
					continue;
				}
	
				if (last_was_cr && sector_buffer[work_int] != '\n')
					(void) fprintf(output, "\r");
	
				(void) fprintf(output, "%c", sector_buffer[work_int]);
	
				last_was_cr = 0;
			}
		}
	} while (bytes_to_read > 0 && (cluster = next_cluster(handle, cluster)) != -1);

	return(0);
}

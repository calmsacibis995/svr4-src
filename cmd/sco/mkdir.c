/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:mkdir.c	1.3"

/* #define		DEBUG		1	/* */

/*
		mkdir(handle, filename, mode)

	Crates the specified directory entry
*/

#include	"MS-DOS.h"

#include	<stdio.h>

#include	<time.h>

Mkdir(handle, loc_filename, mode)
int	handle;
char	*loc_filename;
int	mode;
{
	char	*c_ptr;
	long	cluster;
	unsigned	char	cluster_low;
	unsigned	char	cluster_high;
	unsigned	char	date_low;
	unsigned	char	date_high;
	long	dir_mod_sector;
	int	disp;
	int	i;
	int	j;
	int	in_root = 0;
	int	index;
	long	parent_cluster;
	long	right_now;
	unsigned	char	time_low;
	unsigned	char	time_high;
	struct	tm	*time_ptr;

#ifdef DEBUG
	(void) fprintf(stderr, "mkdir(): DEBUG - mkdir(%d, \"%s\", %d)\n", handle, loc_filename, mode);
#endif

	/*
		Get current time
	*/
	right_now = time((long *) 0);
	time_ptr = localtime(&right_now);

	/*
		Make sure we are all setup properly
	*/
	if ((index = lookup_device(handle)) == -1) {
		(void) fprintf(stderr, "mkdir(): Error - Handle %d not found in device table\n", handle);
		return(-1);
	}

	/*
		Determine where we need to put this file
	*/
	if ((disp = loc_free_dir(handle, loc_filename, mode)) == -1)
		exit(1);

	parent_cluster = SECT_2_CLUS(last_sector_read);

#ifdef DEBUG
	(void) fprintf(stderr, "mkdir(): DEBUG - Parent cluster: %ld Displacement = %d\n", parent_cluster, disp);
#endif

	/*
		last_sector_read is the sector where we must add
		our directory entry, at displacement disp.

		First set first 11 bytes to all blanks
	*/
	dir_mod_sector = last_sector_read;

	for (i = 0; i < 11; i++)
		sector_buffer[disp + i] = ' ';

	/*
		Lay in primary name
	*/
	if (mode == LABEL || strrchr(loc_filename + 1, '/') == NULL)
		in_root = 1;
	else
		in_root = 0;

	if (in_root == 0)
		c_ptr = basename(loc_filename);
	else
		c_ptr = loc_filename;

	if (mode != LABEL && *c_ptr == '/')
		c_ptr++;

#ifdef DEBUG
	(void) fprintf(stderr, "mkdir(): DEBUG - Primary file name: \"");
#endif

	for (i = 0; i < 8 && *(c_ptr + i) != '.' && *(c_ptr + i) != '\0'; ) {

#ifdef DEBUG
		(void) fprintf(stderr, "%c", *(c_ptr + i));
#endif

		sector_buffer[disp + i] = *(c_ptr + i);

		i++;
	}

	for (; *(c_ptr + i) != '.' && *(c_ptr + i) != '\0'; )
		i++;

#ifdef DEBUG
	(void) fprintf(stderr, "\"\n");
#endif

	/*
		If we have an extension - lay it in now
	*/
	if (*(c_ptr + i) == '.') {
		i++;
#ifdef DEBUG
		(void) fprintf(stderr, "mkdir(): DEBUG - Extension: \"");
#endif

		for (j = 8; j < 11 && *(c_ptr + i) != '\0'; i++, j++) {
#ifdef DEBUG
			(void) fprintf(stderr, "%c", *(c_ptr + i));
#endif

			sector_buffer[disp + j] = *(c_ptr + i);
		}
#ifdef DEBUG
		(void) fprintf(stderr, "\"\n");
#endif
	}

	/*
		Next is attribute
	*/
	sector_buffer[disp + FILE_ATTRIBUTE] = mode;

	/*
		Next is time
	*/
	sector_buffer[TIME + disp + 1] = ((time_ptr->tm_hour & 0x1F) << 3);
	sector_buffer[TIME + disp + 1] |= ((time_ptr->tm_min & 0x38) >> 3);
	sector_buffer[TIME + disp] = ((time_ptr->tm_min & 0x07) << 5);

	/*
		Stash the time for future use
	*/
	time_low = sector_buffer[TIME + disp];
	time_high = sector_buffer[TIME + disp + 1];

	/*
		Next is date
	*/
	sector_buffer[DATE + disp + 1] = (((time_ptr->tm_year - 80) & 0x7F) << 1);
	sector_buffer[DATE + disp + 1] |= (((time_ptr->tm_mon + 1) & 0x08) >> 3);
	sector_buffer[DATE + disp] = (((time_ptr->tm_mon + 1) & 0x07) << 5);
	sector_buffer[DATE + disp] |= (time_ptr->tm_mday & 0x1F);

	/*
		Stash the date for future use
	*/
	date_low = sector_buffer[DATE + disp];
	date_high = sector_buffer[DATE + disp + 1];

	/*
		Size is zero - 4 bytes
	*/
	sector_buffer[disp + FILE_SIZE] = '\0';
	sector_buffer[disp + FILE_SIZE + 1] = '\0';
	sector_buffer[disp + FILE_SIZE + 2] = '\0';
	sector_buffer[disp + FILE_SIZE + 3] = '\0';

	sector_buffer[disp + STARTING_CLUSTER] = '\0';
	sector_buffer[disp + STARTING_CLUSTER + 1] = '\0';

	/*
		Need to allocate the first cluster BEFORE we
		touch the directory. This way if we fail, we
		are still ok.
	*/
	if (mode != LABEL) {
		if ((cluster = alloc_cluster(handle)) == -1) {
			(void) fprintf(stderr, "mkdir(): No space on device \"%s\"\n", device_pathname);
			return(-1);
		}
	}

	if (write_sector(handle, last_sector_read) == -1) {
		(void) fprintf(stderr, "mkdir(): Failed to update directory, sector %ld\n\tDisk may be unusable\n", last_sector_read);
		exit(1);
	}

#ifdef DEBUG
	(void) fprintf(stderr, "mkdir(): DEBUG - Added directory entry to sector %ld Displacement: %d\n", last_sector_read, disp);
#endif

	/*
		First cluster is next - LSB is low

		Allocate one, then store its value

		Be careful...Labels get no space!
	*/
	if (mode != LABEL) {
#ifdef DEBUG
		(void) fprintf(stderr, "mkdir(): DEBUG - Mode is not a label (%d)\n", mode);
#endif
	
		sector_buffer[disp + STARTING_CLUSTER + 1] = cluster / 256;
		sector_buffer[disp + STARTING_CLUSTER] = cluster - (sector_buffer[STARTING_CLUSTER + 1] * 256);
	
		/*
			Stash the cluster for future use
		*/
		cluster_low = sector_buffer[disp + STARTING_CLUSTER];
		cluster_high = sector_buffer[disp + STARTING_CLUSTER + 1];
	
		/*
			Add the directory entry for our 
			target file
		*/
		if (write_sector(handle, dir_mod_sector) == -1)
			exit(1);
	
		if (mode == SUB_DIRECTORY) {
			/*
				Add the "." and ".." directory entries
			*/
			if (read_sector(handle, CLUS_2_SECT(cluster)) == -1) {
				(void) fprintf(stderr, "mkdir(): Error - Failed to add the dot and dot-dot directories\n\tdisk may be unusable\n");
				exit(1);
			}
		
			for (i = 0; i < TABLE.bytes_per_sector; i++)
				sector_buffer[i] = 0x00;
		
			for (i = 0; i < 11; i++) {
				sector_buffer[i] = ' ';
				sector_buffer[BYTES_PER_DIR + i] = ' ';
			}
	
			/* Do "." entry */
			sector_buffer[0] = '.';
	
			sector_buffer[FILE_ATTRIBUTE] = SUB_DIRECTORY;
	
			sector_buffer[TIME] = time_low;
			sector_buffer[TIME + 1] = time_high;
	
			sector_buffer[DATE] = date_low;
			sector_buffer[DATE + 1] = date_high;
	
			sector_buffer[STARTING_CLUSTER] = cluster_low;
			sector_buffer[STARTING_CLUSTER + 1] = cluster_high;
	
			/* Do ".." entry */
			sector_buffer[BYTES_PER_DIR] = '.';
			sector_buffer[BYTES_PER_DIR + 1] = '.';
	
			sector_buffer[BYTES_PER_DIR + FILE_ATTRIBUTE] = SUB_DIRECTORY;
	
			sector_buffer[BYTES_PER_DIR + TIME] = time_low;
			sector_buffer[BYTES_PER_DIR + TIME + 1] = time_high;
	
			sector_buffer[BYTES_PER_DIR + DATE] = date_low;
			sector_buffer[BYTES_PER_DIR + DATE + 1] = date_high;
	
			sector_buffer[STARTING_CLUSTER + BYTES_PER_DIR + 1] = in_root ? 0x00 : (parent_cluster / 256);
			sector_buffer[STARTING_CLUSTER + BYTES_PER_DIR] = in_root ? 0x00 : (parent_cluster - (sector_buffer[STARTING_CLUSTER + BYTES_PER_DIR + 1] * 256));
	
			/*
				Write out updated directory sector
			*/
			if (write_sector(handle, CLUS_2_SECT(cluster)) == -1) {
				(void) fprintf(stderr, "mkdir(): Error - Failed to add the dot and dot-dot directories\n\tdisk may be unusable\n");
				exit(1);
			}
		}
	}
	return(disp);
}

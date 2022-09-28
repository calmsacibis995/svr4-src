/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:loc_free_dir.c	1.3"

/* #define		DEBUG		1	/* */
/* #define		STANDALONE	1

/*
			loc_free_dir()

	Locates the next free directory entry for a passed target file.

	For example, should we wish to create a file /a/b/c, we would
	pass /a/b/c to this routine and it will set last_sector_read
	to the sector where the free slot was found (in /a/b). It
	returns the displacement into that sector of the free entry.

	A return value of -1 indicates failure, and last_sector_read
	is not guaranteed.
*/

#include	"MS-DOS.h"

#include	<stdio.h>

#include	<fcntl.h>

loc_free_dir(handle, pathname, mode)
int	handle;
char	*pathname;
int	mode;
{
	char	*c_ptr;
	long	cluster;
	int	disp;
	int	in_root = 0;
	long	loc_sector;
	int	i;
	int	index;
	long	w_sector;

	/*
		Locate this handle in our device table
	*/
	if ((index = lookup_device(handle)) == -1) {
		(void) fprintf(stderr, "loc_free_dir(): Error - Handle %d not found in device table\n", handle);
		return(-1);
	}

	/*
		If we have slashes in our arg, then
		we want to truncate (temporarily) at
		the last slash, and locate on that, otherwise
		we must work in root.
	*/
	if (mode != LABEL && (c_ptr = strrchr(pathname + 1, '/')) != NULL) {
		*c_ptr = '\0';

		if ((disp = locate(handle, pathname)) == -1) {
			(void) fprintf(stderr, "loc_free_dir: File \"%s\" not found\n", pathname);
			return(-1);
		}

		loc_sector = CLUS_2_SECT(GET_CLUS(disp));
		*c_ptr = '/';
	} else {
		in_root = 1;
	}

	/*
		loc_sector points to the directory in which
		we need to find an empty slot.
	*/
	if (in_root) {
		/*
			Search root directory
		*/
		for (w_sector = TABLE.root_base_sector; w_sector < TABLE.root_base_sector + TABLE.sectors_in_root; w_sector++) {
			if (read_sector(handle, w_sector) == -1)
				exit(1);

			for (disp = 0; disp < TABLE.bytes_per_sector; disp += BYTES_PER_DIR) {
				if (sector_buffer[disp] == 0xE5 || sector_buffer[disp] == 0x00)
					return(disp);
			}
		}

		(void) fprintf(stderr, "loc_free_dir(): Error - Root directory on device \"%s\" is full\n", device_pathname);
		return(-1);
	}
	else {
		w_sector = loc_sector;

		for (;;) {
			/*
				Search a sub directory
			*/
			for (; w_sector < loc_sector + TABLE.sectors_per_cluster; w_sector++) {
				if (read_sector(handle, w_sector) == -1)
					exit(1);
	
				for (disp = 0; disp < TABLE.bytes_per_sector; disp += BYTES_PER_DIR) {
					if (sector_buffer[disp] == 0xE5 || sector_buffer[disp] == 0x00)
						return(disp);
				}
			}
	
			/*
				We reached end of cluster. Need
				to get next one - if one is
				available.
			*/
			we_are_dosrm = 1;

			w_sector--;

			if ((cluster = next_cluster(handle, SECT_2_CLUS(w_sector))) == -1) {
#ifdef DEBUG
				(void) fprintf(stderr, "loc_free_dir: Need to allocate another cluster\n");
#endif
				if ((cluster = alloc_cluster(handle)) == -1) {
					(void) fprintf(stderr, "loc_free_dir: No space left on device \"%s\"\n", device_pathname);
					exit(1);
				}
	
				if (chain_cluster(handle, cluster, SECT_2_CLUS(w_sector)) == -1)
					exit(1);

				if (chain_cluster(handle, (long) (TABLE.fat16 ? 0xFFFF : 0xFFF), cluster) == -1)
					exit(1);

				if (write_fat(handle) == -1)
					exit(1);

				if (read_sector(handle, CLUS_2_SECT(cluster)) == -1)
					exit(1);

				for (i = 0; i < TABLE.bytes_per_sector; i++)
					sector_buffer[i] = '\0';

#ifdef DEBUG
				fprintf(stderr, "loc_free_dir(): DEBUG - Writing sector number %ld, cluster %ld\n", (long) (CLUS_2_SECT(cluster)), cluster);
#endif

				if (write_sector(handle, (long) (CLUS_2_SECT(cluster))) == -1)
					exit(1);

				if (read_sector(handle, (long) (CLUS_2_SECT(cluster))) == -1)
					exit(1);

				return(0);
				
			}
	
			w_sector = loc_sector = (long) (CLUS_2_SECT(cluster));
		}
	}
}

#ifdef STANDALONE
main(argc, argv)
int	argc;
char	**argv;
{
	int	handle;
	int	index;
	int	disp;

	/*
		Locate this handle in our device table
	*/
	if ((handle = open_device(argv[1], O_RDONLY)) == -1)
		exit(1);

	if ((index = lookup_device(handle)) == -1) {
		(void) fprintf(stderr, "loc_free_dir(): Invalid device handle\n");
		return(-1);
	}

	if ((disp = loc_free_dir(handle, filename)) == -1) {
		(void) fprintf(stderr, "loc_free_dir(%d, \"%s\") failed\n");
		exit(1);
	}

	printf("File \"%s\" would be created in sector %ld, at displacement %d\n", argv[1], last_sector_read, disp);

	(void) close_device(handle);
}
#endif

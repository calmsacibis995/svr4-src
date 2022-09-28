/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:locate.c	1.3"

/* #define	DEBUG		1	/* */

/*
			locate(file, index)

	Pass this routine a filename and an index into our device_table
	and it will load the sector containing the directory entry
	for the specified file, and return the displacement into the 
	sector of its directory entry.

	Return -1 if not found;
*/

#include	"MS-DOS.h"

#include	<stdio.h>

locate(handle, loc_filename)
int	handle;
char	*loc_filename;
{
	char	*c_ptr1;
	char	*c_ptr2;
	long	cluster;
	long	current_dir;
	int	disp;
	int	index;
	char	*last_c_ptr2;
	char	locw_filename[MAX_FILENAME];
	char	w_filename[MAX_FILENAME];

	(void) strcpy(locw_filename, loc_filename);

#ifdef DEBUG
	(void) fprintf(stderr, "locate(): Handle: %d loc_filename: \"%s\"\n", handle, loc_filename);
#endif
	/*
		Locate this handle in our device table
	*/
	if ((index = lookup_device(handle)) == -1) {
		(void) fprintf(stderr, "locate(): Error - Handle %d not found in device table\n", handle);
		return(-1);
	}

	/*
		Names like "a/b" mess us up.
		Compensate here by changin them to 
			"/a/b".
	*/
	if ((c_ptr1 = strchr(locw_filename, '/')) != NULL && *locw_filename != '/') {
		(void) sprintf(w_filename, "/%s", locw_filename);
		(void) strcpy(locw_filename, w_filename);
	}

	current_dir = TABLE.root_base_sector;

	if (strchr(locw_filename + 1, '/') == NULL) {
#ifdef DEBUG
		(void) fprintf(stderr, "locate(): DEBUG - Calling scan_dos_dir(%d, %ld, %d, \"%s\") #1\n", handle, current_dir, 0, *locw_filename == '/' ? locw_filename + 1 : locw_filename);
#endif
		disp = scan_dos_dir(handle, current_dir, 0, *locw_filename == '/' ? locw_filename + 1 : locw_filename);
	}
	else {
		/*
			If sector_buffer has changed, reload
		*/
		if (last_sector_read != current_dir)
			if (read_sector(handle, current_dir) == -1)
				return(-1);

		for (c_ptr1 = locw_filename; (c_ptr1 = strchr(c_ptr1, '/')) != NULL; c_ptr1++) {
			if ((c_ptr2 = strchr(c_ptr1 + 1, '/')) != NULL) {
				*c_ptr2 = '\0';
#ifdef DEBUG
				(void) fprintf(stderr, "locate(): DEBUG - Traversing \"%s\"\n", c_ptr1 + 1);
#endif
				/*
					Try to locate the file
				*/
#ifdef DEBUG
				(void) fprintf(stderr, "locate(): DEBUG - Calling scan_dos_dir(%d, %ld, %d, \"%s\") #2\n", handle, current_dir, 0, c_ptr1 + 1);
#endif
				if ((disp = scan_dos_dir(handle, current_dir, 0, c_ptr1 + 1)) == -1) {
					*c_ptr2 = '/';
					return(-1);
				}

				cluster = GET_CLUS(disp);

				current_dir = CLUS_2_SECT(cluster);

#ifdef DEBUG
				(void) fprintf(stderr, "locate(): DEBUG - File \"%s\" is in cluster %ld == sector %ld\n", c_ptr1 + 1, cluster, current_dir);
#endif
				*c_ptr2 = '/';
				last_c_ptr2 = c_ptr2 + 1;
			}
		}

#ifdef DEBUG
		(void) fprintf(stderr, "locate(): DEBUG - Calling scan_dos_dir(%d, %ld, %d, \"%s\") #3\n", handle, current_dir, 0, last_c_ptr2);
#endif
		if ((disp = scan_dos_dir(handle, current_dir, 0, last_c_ptr2)) == -1)
			return(-1);

#ifdef DEBUG
		(void) fprintf(stderr, "locate(): DEBUG - File \"%s\" is in cluster %ld == sector %ld #2\n", last_c_ptr2, GET_CLUS(disp), CLUS_2_SECT(GET_CLUS(disp)));
#endif
	}

	return(disp);
}

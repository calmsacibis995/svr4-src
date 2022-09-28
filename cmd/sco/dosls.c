/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:dosls.c	1.3"

/*
		dosls
*/

#include	<stdio.h>
#include	"MS-DOS.h"

#include	<fcntl.h>

main(argc, argv)
int	argc;
char	**argv;
{
	int	disp;
	int	handle;
	int	j;
	int	index;
	int	rc = 0;
	int	total_files = 0;

	if (argc == 1) {
		(void) fprintf(stderr, "Usage: %s directory...\n", argv[0]);
		exit(1);
	}

#ifdef DOSDIR
	we_are_dosdir = 1;
#endif
	

#ifdef DOSDIR
	for (j = 1; j < argc; j++) {
#else
	j = 1;
#endif
		if ((handle = open_device(argv[j], O_RDONLY)) == -1)
			exit(1);
	
		/*
			Locate this handle in our device table
		*/
		if ((index = lookup_device(handle)) == -1) {
			(void) fprintf(stderr, "dosls: Error - Handle %d not found in device table\n", handle);
			return(-1);
		}

		/*
			If we are MS-DOS dir format - display 
			volume label
		*/
		if (we_are_dosdir)
			if (disp_vol(handle, filename) == -1)
				exit(1);

		if (*filename == '\0' || strcmp(filename, "/") == 0) {
			if ((total_files = disp_dos_dir(handle, TABLE.root_base_sector, 0)) == -1)
				rc = 1;
		}
		else if ((disp = locate(handle, filename)) == -1) {
			(void) fprintf(stderr, "File not found\n");
		}
		else if (sector_buffer[FILE_ATTRIBUTE + disp] == SUB_DIRECTORY) {
			if ((total_files = disp_dos_dir(handle, (long) CLUS_2_SECT(GET_CLUS(disp)), 0)) == -1) 
				rc = 1;
		}
		else {
			(void) fprintf(stderr, "File \"%s\" not a directory\n", argv[j]);
			rc = 1;
		}

		if (we_are_dosdir && rc == 0)
			(void) printf(" %9d File(s) %9ld Bytes free\n", total_files, free_space(handle));

		(void) close_device(handle);
#ifdef DOSDIR
	}
#endif

	exit(rc);	/* NOTREACHED */
}

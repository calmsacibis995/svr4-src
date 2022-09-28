/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:dosrm.c	1.3"

/* #define	DEBUG		1	/* */

/*
		dosrm

	Deletes an MS-DOS file under UNIX.
*/

#include	"MS-DOS.h"

#include	<stdio.h>
#include	<fcntl.h>

main(argc, argv)
int	argc;
char	**argv;
{
	int	c;
	long	disp;
	int	handle;

	we_are_dosrm = 1;

#ifdef DOSRMDIR
	we_are_dosrmdir = 1;
#endif

	/*
		Loop over our list of files, deleting as we go
	*/
	for (c = 1; c < argc; c++) {
		/*
			Open device using our interface.
		*/
		if ((handle = open_device(argv[c], O_RDWR)) == -1)
			exit(1);
	
		/*
			Make sure we are all setup properly
		*/
		if (lookup_device(handle) == -1) {
			(void) fprintf(stderr, "dosrm: Failed to locate handle %d in device_table\n", handle);
			exit(1);
		}

		if ((disp = locate(handle, filename)) == -1)
			(void) fprintf(stderr, "File \"%s\" not found\n", argv[c]);
		else {
#ifdef DOSRMDIR
			if (sector_buffer[FILE_ATTRIBUTE + disp] == SUB_DIRECTORY) 
#else
			if (sector_buffer[FILE_ATTRIBUTE + disp] != SUB_DIRECTORY) 
#endif
			{
				dir_sector = last_sector_read;

				if (rm_file(handle, disp) == -1) {
#ifdef DOSRMDIR
					(void) fprintf(stderr, "Directory \"%s\" not empty\n", argv[c]);
					continue;
#endif
				}
			} else {
#ifdef DOSRMDIR
				(void) fprintf(stderr, "\"%s\" is not a directory\n", argv[c]);
#else
				(void) fprintf(stderr, "\"%s\" directory\n", argv[c]);
#endif
			}
		}

		/*
			Close our files, using our interface
		*/
		(void) close_device(handle);
	}


	exit(0);	/* NOTREACHED */
}

/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:dosmkdir.c	1.3"

/* #define	DEBUG		1	/* */

/*
		dosmkdir

	Deletes an MS-DOS file under UNIX.
*/

#include	"MS-DOS.h"

#include	<stdio.h>

#include	<fcntl.h>

#include	<time.h>


main(argc, argv)
int	argc;
char	**argv;
{
	int	c;
	int	handle;

	/*
		Loop over our list of files, creating 
		directories as we go.
	*/
	for (c = 1; c < argc; c++) {
		/*
			Open device using our interface.

			We are critical until file is closed.
		*/
		critical(1);

		if ((handle = open_device(argv[c], O_RDWR)) == -1)
			exit(1);
	
		/*
			Make sure we are all setup properly
		*/
		if (lookup_device(handle) == -1) {
			(void) fprintf(stderr, "dosmkdir: Failed to locate handle %d in device_table\n", handle);
			(void) close_device(handle);
			exit(1);
		}

		/*
			Do not alow creation of root directory
		*/
		if (*filename == '\0') {
			(void) fprintf(stderr, "dosmkdir: Error - Can't make root directory on \"%s\"\n", device_pathname);
			(void) close_device(handle);
			exit(1);
		}

		/*
			Make sure the requested file does
			not alreay exist.
		*/
		if (locate(handle, filename) != -1) {
			(void) fprintf(stderr, "dosmkdir: \"%s\" already exists\n", argv[c]);
			continue;
		}

		if (Mkdir(handle, filename, SUB_DIRECTORY) == -1) 
			(void) fprintf(stderr, "dosmkdir: mkdir(%d, \"%s\") failed\n", handle, filename);

		if (write_fat(handle) == -1) {
			(void) fprintf(stderr, "Failed to update FAT\n\tDisk may be unusable\n");
		}

		/*
			Close our files, using our interface

			No longer in critical code
		*/
		(void) close_device(handle);

		critical(0);
	}

	exit(0);	/* NOTREACHED */
}

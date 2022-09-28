/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:doslabel.c	1.3"

/* #define	DEBUG		1	/* */

/*
		doslabel

	Create/Remove a label from an MS-DOS volume
*/

#include	"MS-DOS.h"

#include	<stdio.h>
#include	<fcntl.h>

static	char	*usage = "Usage: doslabel drive\n";

main(argc, argv)
int	argc;
char	**argv;
{
	char	buffer[20];
	int	handle;

	if (argc != 2) {
		(void) fprintf(stderr, usage);
		exit(1);
	}

	/*
		Must be an MS-DOS file

		Open device using our interface.
	*/
	if ((handle = open_device(argv[1], O_RDWR)) == -1)
		exit(1);

	/*
		Make sure we are all setup properly
	*/
	if (lookup_device(handle) == -1) {
		(void) fprintf(stderr, "doslabel: Failed to locate handle %d in device_table\n", handle);
		exit(1);
	}

	(void) printf("\nVolume label (11 characters, <RETURN> for none)? ");
	(void) fflush(stdout);

	if (fgets(buffer, 20, stdin) != NULL) 
		(void) make_label(handle, buffer);

	/*
		Close our files, using our interface
	*/
	(void) close_device(handle);

	exit(0);	/* NOTREACHED */
}

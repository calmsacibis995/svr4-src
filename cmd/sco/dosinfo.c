/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:dosinfo.c	1.3"

/* #define		DEBUG		1	/* */

/*
	Open a specified device in the specified mode.
	Return handle on success,. -1 on failure.
*/

#include	"MS-DOS.h"

#include	<stdio.h>

#include	<fcntl.h>

main(argc, argv)
int	argc;
char	**argv;
{
	we_are_dosinfo = 1;

	if (argc != 2) {
		(void) fprintf(stderr, "Usage: dosinfo drive\n");
		exit(1);
	}

	/*
		Open and close the device
	*/
	(void) close_device(open_device(argv[1], O_RDONLY));
	exit(0);	/* NOTREACHED */
}

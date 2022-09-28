/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:fatlist.c	1.3"

#include	"MS-DOS.h"

#include	<stdio.h>

#include	<fcntl.h>

main(argc, argv)
int	argc;
char	**argv;
{
	int	handle;
	int	i;
	int	index;
	long	j;
	long	value;

	for (i = 1; i < argc; i++) {
		if ((handle = open_device(argv[i], O_RDONLY)) == -1)
			exit(1);

		/*
			Locate this handle in our device table
		*/
		if ((index = lookup_device(handle)) == -1) {
			(void) fprintf(stderr, "fatlist: Error - Handle %d not found in device table\n", handle);
			exit(1);
		}

		for (j = 2; j < TOTAL_CLUSTERS; j++)
			if (next_cluster(handle, j) == -1)
				(void) printf("\n");
			else {
				value = next_cluster(handle, j);

				(void) printf("%6ld - 0x%-4lX		%4ld - 0x%-4lX\n", j, j, value, value);
				(void) printf("---------------------------------------\n");
			}
	}

	exit(0);	/* NOTREACHED */
}

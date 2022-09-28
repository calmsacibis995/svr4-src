/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:fdread.c	1.3"

/*
		fdread

	Does direct diskette reads.

	usage:	fdread sector
*/

#include	<stdio.h>
#include	"MS-DOS.h"

#include	<fcntl.h>

main(argc, argv)
int	argc;
char	**argv;
{
	int	handle;
	int	i;
	int	index;

	/*
		Make sure we have 2 arguments
	*/
	if (argc != 3) {
		(void) fprintf(stderr, "Usage: fdread DRIVE SECTOR\n");
		(void) fprintf(stderr, "\n\tUse this command to do direct reads\n");
		(void) fprintf(stderr, "\tof MS-DOS formatted floppy media.\n");
		(void) fprintf(stderr, "\tSupply it a sector number, and it dumps\n");
		(void) fprintf(stderr, "\tthe contrents of that sector to stdout.\n\n");
		exit(1);
	}

	if ((handle = open_device(argv[1], O_RDONLY)) != -1) {
		if ((i = lookup_device(handle)) == -1) {
			(void) fprintf(stderr, "fdread: Failed to locate handle in device_table\n");
			exit(1);
		}

		if ((index = lookup_device(handle)) == -1) {
			(void) fprintf(stderr, "fdread: Error - Handle %d not found in device table\n", handle);
			exit(1);
		}

		if (read_sector(handle, (long) atoi(argv[2])) == -1)
			exit(1);

		/*
			Dump the read sector
		*/
		for (i = 0; i <= TABLE.bytes_per_sector; i++) {
			if (i < TABLE.bytes_per_sector) {
#ifdef HEX_DISPLAY
				if (i % 8 == 0)
					(void) printf("\n%d	", i);

				(void) printf("%3x  ", sector_buffer[i]);
#else
				(void) printf("%c", sector_buffer[i]);
#endif
			}
		}

		(void) printf("\n");

		(void) close_device(handle);
	}

	exit(0);	/* NOTREACHED */
}

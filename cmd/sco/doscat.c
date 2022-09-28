/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:doscat.c	1.3"

/* #define	DEBUG		1	/* */

/*
		doscat

	Reads an MS-DOS file under UNIX.
*/

#include	"MS-DOS.h"

#include	<stdio.h>
#include	<fcntl.h>

static	char	*usage = "Usage: doscat [-r | -m] file ...\n";

main(argc, argv)
int	argc;
char	**argv;
{
	int	c;
	int	conversion;
	int	disp;
	int	handle;

	while ((c = getopt(argc, argv, "rm")) != EOF)
		switch(c) {
		case 'm':
			conversion = 1;
			break;

		case 'r':
			conversion = 0;
			break;

		default:
			(void) fprintf(stderr, usage);
			exit(1);
		}

	if (optind >= argc) {
		(void) fprintf(stderr, usage);
		exit(1);
	}


	/*
		Start at the root directory
	*/
	for (c = optind; c < argc; c++) {
		/*
			Is this file a UNIX file?
		*/
		if (strchr(argv[c], ':') == NULL) {
			/* Yes, UNIX file, use /bin/cat */
			char	cmd_buffer[MAX_FILENAME];

			(void) sprintf(cmd_buffer, "/bin/cat %s", argv[c]);
			(void) system(cmd_buffer);
			continue;
		}

		/*
			Must be an MS-DOS file

			Open device using our interface.
		*/
		if ((handle = open_device(argv[c], O_RDONLY)) == -1)
			exit(1);
	
		/*
			Make sure we are all setup properly
		*/
		if (lookup_device(handle) == -1) {
			(void) fprintf(stderr, "doscat: Failed to locate handle %d in device_table\n", handle);
			exit(1);
		}

		if ((disp = locate(handle, filename)) == -1)
			(void) fprintf(stderr, "File \"%s\" not found\n", filename);
		else
			(void) read_file(handle, disp, conversion, stdout);

		/*
			Close our files, using our interface
		*/
		(void) close_device(handle);
	}

	exit(0);	/* NOTREACHED */
}

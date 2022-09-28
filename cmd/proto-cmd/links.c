/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)proto-cmd:links.c	1.3"

#include <errno.h>

main(argc, argv)
int argc;
char *argv[];
{
	char buf[200];
	int x;
	if (access(argv[1], 00) < 0)
		exit (1);
	for (x=2;x < argc; x++) {
		if (access(argv[x], 00) == 0)
			unlink (argv[x]);
		errno = 0;
		if (link(argv[1],argv[x]) < 0)
		   if (errno == EXDEV) {
			strcpy (buf,"cp ");
			strcat (buf, argv[1]);
			strcat (buf," ");
			strcat (buf, argv[x]);
			system (buf);
		}
	}
	exit (0);
}

/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucblibc:port/gen/mkstemp.c	1.1.3.1"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 

#include <sys/fcntl.h>

mkstemp(as)
	char *as;
{
	register char *s;
	register unsigned int pid;
	register int fd, i;

	pid = getpid();
	s = as;
	while (*s++)
		/* void */;
	s--;
	while (*--s == 'X') {
		*s = (pid % 10) + '0';
		pid /= 10;
	}
	s++;
	i = 'a';
	while ((fd = open(as, O_CREAT|O_EXCL|O_RDWR, 0600)) == -1) {
		if (i == 'z')
			return(-1);
		*s = i++;
	}
	return(fd);
}

/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1984, 1986, 1987, 1988, 1989 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libns:rfrcv.c	1.1.4.1"

#include "tiuser.h"

/*
 * substitute for t_rcv to read in a pseudo-byte stream fashion.
 */

int
rf_rcv(fd, bufp, bytes, flagp)
int fd;
char *bufp;
int bytes;
int *flagp;
{
	register int n;
	register int count = bytes;
	register char *bp = bufp;

	*flagp = 0;

	do {
		n = t_rcv(fd, bp, count, flagp);
		if (n < 0)
			return(n);
		count -= n;
		bp += n;
	} while (count > 0);

	return(bp - bufp);
}

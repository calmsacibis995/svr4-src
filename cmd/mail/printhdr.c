/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:printhdr.c	1.3.3.1"
#include "mail.h"

printhdr (type, hdrtype, hptr, fp)
int		hdrtype;
struct hdrs	*hptr;
FILE		*fp;
{
	char		buf[1024];
	register int	n;
	struct hdrs 	*contptr;

	if (sel_disp(type, hdrtype, header[hdrtype].tag) < 0) {
		return (0);
	}

	sprintf(buf,"%s %s\n", header[hdrtype].tag, hptr->value);
	n = strlen (buf);
	if (fwrite(buf,1,n,fp) != n)  {
		sav_errno = errno;
		return(-1);
	}

	/* Print continuation lines, if any... */
	contptr = hptr;
	while (contptr->cont != (struct hdrs *)NULL) {
		contptr = contptr->cont;
		sprintf(buf,"%s\n",contptr->value);
		n = strlen (buf);
		if (fwrite(buf,1,n,fp) != n)  {
			sav_errno = errno;
			return(-1);
		}
	}
	return (0);
}

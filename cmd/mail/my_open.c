/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:my_open.c	1.3.3.1"
#include "mail.h"

#undef open()
#undef close()
#undef fopen()
#undef fclose()

my_open(s,n)
char *s;
int n;
{
	int	fd;

	fd = open(s,n);

	Dout(pn, 0, "fd = %d, filename = '%s'\n", fd, s);
	return (fd);
}

my_close(n)
int	n;
{
	int rc;

	rc = close(n);

	Dout(pn, 0, "fd = %d, rc from close() = %d\n", n, rc);
	return (rc);
}

FILE *
my_fopen(s,t)
char *s, *t;
{
	FILE	*fp;

	fp = fopen(s,t);

	Dout(pn, 0, "fd = %d, filename = '%s'\n", fileno(fp), s);
	return (fp);
}

my_fclose(fp)
FILE	*fp;
{
	int rc, fd;

	fd = fileno(fp);

	rc = fclose(fp);

	Dout(pn, 0, "fd = %d, rc from fclose() = %d\n", fd, rc);
	return (rc);
}

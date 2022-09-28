/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:smtp/src/netio.c	1.3.3.1"
#ifndef lint
static char *sccsid = "@(#)netio.c	1.7 87/07/31";
#endif

#include <stdio.h>
#include <setjmp.h>

#include "smtp.h"
#include "miscerrs.h"

#ifdef NOBOMB
#define bomb exit
#endif

char *strcpy(), *strncat();
extern int debug;

int
tgets(line, size, fi)		/* fgets from TCP */
char *line;
int size;
FILE *fi;
{
	register char *cr, *rp;

	*line = 0;
	rp = fgets(line, size, fi);
	if (ferror(fi) || rp==NULL) {
		perror("error reading from smtp");
		bomb(E_IOERR);
	}

	/* convert \r\n -> \n */
	cr = line + strlen(line) - 2;
	if (cr >= line && *cr == '\r' && *(cr+1) == '\n') {	/* CRLF there? */
		*cr++ = '\n';
		*cr = 0;
	} else				/* no CRLF present */
		cr += 2;		/* point at NUL byte */

	if(debug)
		(void) fprintf(stderr, "<<< %s", line);
	if (feof(fi)) {
		perror("read eof from smtp");
		bomb(E_IOERR);
	}
	return cr - line;
}

int
tputs(line, fo)			/* fputs to TCP */
char *line;
FILE *fo;
{
	char buf[MAXSTR];
	register char *nl;
	extern int debug;

	(void) strcpy(buf, line);
	if(debug)
		(void) fprintf(stderr, ">>> %s", buf);
	/* replace terminating \n by \r\n */
	nl = buf + strlen(buf) - 1;		/* presumably \n */
	if (nl >= buf && *nl=='\n') {		/* if it is ... */
		*nl++ = '\r';
		*nl++ = '\n';
		*nl = 0;
	} else
		printf("unterminated line: <%s>\n", buf);

	(void) fputs(buf, fo);
	(void) fflush(fo);
	if (ferror(fo)) {
		perror("error writing to smtp");
		bomb(E_IOERR);
	}
	return 0;
}

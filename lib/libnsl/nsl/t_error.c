/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libnsl:nsl/t_error.c	1.2.2.1"
#include "sys/stropts.h"
#include "sys/tiuser.h"
#include "sys/errno.h"
#include "_import.h"

extern int t_errno, strlen(), write();
extern  char *t_errlist[];
extern t_nerr;
extern void perror();

void
t_error(s)
char	*s;
{
	register char *c;
	register int n;

	c = "Unknown error";
	if(t_errno <= t_nerr)
		c = t_errlist[t_errno];
	n = strlen(s);
	if(n) {
		(void) write(2, s, (unsigned)n);
		(void) write(2, ": ", 2);
	}
	(void) write(2, c, (unsigned)strlen(c));
	if (t_errno == TSYSERR) {
		(void) write(2, ": ", 2);
		perror("");
	} else
		(void) write(2, "\n", 1);
}

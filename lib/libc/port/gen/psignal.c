/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/psignal.c	1.1"

/*
 * Print the name of the signal indicated by "sig", along with the
 * supplied message
 */

#ifdef __STDC__
	#pragma weak psignal = _psignal
#endif
#include	"synonyms.h"
#include	<signal.h>

extern char *_sys_siglist[];

void
psignal(sig, s)
char	*s;
{
	register char *c;
	register int n;

	if (sig < 0 || sig >= NSIG)
		sig = 0;
	c = _sys_siglist[sig];
	n = strlen(s);
	if(n) {
		(void) write(2, s, (unsigned)n);
		(void) write(2, ": ", 2);
	}
	(void) write(2, c, (unsigned)strlen(c));
	(void) write(2, "\n", 1);
}

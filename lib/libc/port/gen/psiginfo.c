/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/psiginfo.c	1.2"

/*
 * Print the name of the siginfo indicated by "sig", along with the
 * supplied message
 */

#ifdef __STDC__
	#pragma weak psiginfo = _psiginfo
#endif
#include	"synonyms.h"
#include	<signal.h>
#include	<siginfo.h>

void
psiginfo(sip, s)
siginfo_t *sip;
char	*s;
{
	char buf[16];
	register char *c;
	struct siginfolist *listp;

	if (sip == 0)
		return;

	(void) write(2, s, (unsigned)strlen(s));
	(void) write(2, ": ", 2);
	c =  _sys_siglist[sip->si_signo];
	(void) write(2, c, (unsigned)strlen(c));
	if (sip->si_code == 0) {
		(void) write(2, " ( from process ", 16);
		sprintf(buf," %d",sip->si_pid);
		(void) write(2, buf, (unsigned)strlen(buf));
		(void) write(2, " )", 2);
	}
	else if ((listp = &_sys_siginfolist[sip->si_signo-1]) 
	  && sip->si_code > 0
	  && sip->si_code <= listp->nsiginfo) {
		c = listp->vsiginfo[sip->si_code-1];
		(void) write(2, " (", 2);
		switch (sip->si_signo) {
			case SIGSEGV:
			case SIGBUS:
			case SIGILL:
			case SIGFPE:
				sprintf(buf," [%x] ",sip->si_addr);
				(void) write(2, buf, (unsigned)strlen(buf));
				break;
		}
		(void) write(2, c, (unsigned)strlen(c));
		(void) write(2, ")", 1);
	}
	(void) write(2, "\n", 1);
}

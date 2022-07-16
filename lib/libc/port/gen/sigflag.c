/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/sigflag.c	1.1"

/* change state of signal flag */

#ifdef __STDC__
	#pragma weak sigflag = _sigflag
#endif
#include "synonyms.h"
#include <signal.h>

sigflag(sig, flag, on)
int sig, on, flag;
{
	struct sigaction sa;
	register v;
	if ((v = sigaction(sig, 0, &sa)) < 0)
		return (v);
	if (on)
		sa.sa_flags |= flag;
	else
		sa.sa_flags &= ~flag;
	return sigaction(sig, &sa, 0);
}

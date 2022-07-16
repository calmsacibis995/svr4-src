/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/hrtcftime.c	1.4"

#include "synonyms.h"
#include <time.h>
#include <limits.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/dl.h>
#include <sys/evecb.h>
#include <sys/hrtcntl.h>

extern char	*getenv();

int
_hrtcftime(bufp, fmtp, tdp)
char *bufp;
char *fmtp;
hrtime_t *tdp;
{
	register struct tm	*tmp;
	
	if (tdp->hrt_res <= 0 || tdp->hrt_res > NANOSEC || tdp->hrt_rem <0)
		return(-1);

	if (tdp->hrt_rem >= tdp->hrt_res) {
		tdp->hrt_secs 	+= tdp->hrt_secs / tdp->hrt_res;
		tdp->hrt_rem	 = tdp->hrt_secs % tdp->hrt_res;
	}

	tmp = localtime((long *) &tdp->hrt_secs);
	return(_hrtascftime(bufp, fmtp, tmp, tdp->hrt_rem, tdp->hrt_res));
} 

int
_hrtascftime(buf, format, tm, rem, res)
char *buf;
char *format;
struct tm *tm;
ulong rem;
ulong res;
{
	/* Set format string, if not already set */
	if (format == NULL || *format == '\0')
		if (((format = getenv("CFTIME")) == 0) || *format == 0)
			format =  "%c";

	return(_hrtstrftime(buf, LONG_MAX, format, tm, rem, res));
}

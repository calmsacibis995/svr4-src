/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/bsd/gethostnm.c	1.3.2.1"

#include <unistd.h>
#include <string.h>
#if defined(__STDC__)
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include <sys/utsname.h>
#include "lpd.h"

char *
#if defined (__STDC__)
gethostname(void)
#else
gethostname()
#endif
{
	struct utsname	utsname;
	static char 	lhost[HOSTNM_LEN];

	if (uname(&utsname) < 0)
		return(NULL);
	strncpy(lhost, utsname.nodename, sizeof(lhost));
	return(lhost);
}

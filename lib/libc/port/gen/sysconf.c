/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/sysconf.c	1.6"

/* sysconf(3C) - returns system configuration information
*/

#ifdef __STDC__
	#pragma weak sysconf = _sysconf
#endif
#include "synonyms.h"
#include <sys/types.h>
#include <unistd.h>
#include <sys/sysconfig.h>
#include <sys/errno.h>
#include <limits.h>
#include <time.h>

extern int errno;

long
sysconf(name)
int name;
{

	switch(name) {
		default:
			errno = EINVAL;
			return(-1);

		case _SC_ARG_MAX:
			return(ARG_MAX);

		case _SC_CLK_TCK:
			return(_sysconfig(_CONFIG_CLK_TCK));

		case _SC_JOB_CONTROL:
			return(_POSIX_JOB_CONTROL);

		case _SC_SAVED_IDS:
			return(_POSIX_SAVED_IDS);

		case _SC_CHILD_MAX:
			return(_sysconfig(_CONFIG_CHILD_MAX));

		case _SC_NGROUPS_MAX:
			return(_sysconfig(_CONFIG_NGROUPS));

		case _SC_OPEN_MAX:
			return(_sysconfig(_CONFIG_OPEN_FILES));

		case _SC_VERSION:
			return(_sysconfig(_CONFIG_POSIX_VER));

		case _SC_PAGESIZE:
			return(_sysconfig(_CONFIG_PAGESIZE));
	
		case _SC_XOPEN_VERSION:
			return(_sysconfig(_CONFIG_XOPEN_VER));

		case _SC_PASS_MAX:
			return(PASS_MAX);

		case _SC_LOGNAME_MAX:
			return(LOGNAME_MAX);
	}
}


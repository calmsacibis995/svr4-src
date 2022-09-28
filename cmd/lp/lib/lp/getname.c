/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/lp/getname.c	1.8.3.1"
/*
 *	getname(name)  --  get logname
 *
 *		getname tries to find the user's logname from:
 *			${LOGNAME}, if set and if it is telling the truth
 *			/etc/passwd, otherwise
 *
 *		The logname is returned as the value of the function.
 *
 *		Getname returns the user's user id converted to ASCII
 *		for unknown lognames.
 *
 */

#include "string.h"
#include "pwd.h"
#include "errno.h"
#include "sys/types.h"
#include "stdlib.h"
#include "unistd.h"

#include "lp.h"

char *
#if	defined(__STDC__)
getname (
	void
)
#else
getname ()
#endif
{
	uid_t			uid;
	struct passwd		*p;
	static char		*logname	= 0;
	char			*l;

	if (logname)
		return (logname);

	uid = getuid();

	setpwent ();
	if (
		!(l = getenv("LOGNAME"))
	     || !(p = getpwnam(l))
	     || p->pw_uid != uid
	)
		if ((p = getpwuid(uid)))
			l = p->pw_name;
		else
			l = 0;
	endpwent ();

	if (l)
		logname = Strdup(l);
	else {
		if (uid > 0) {
			logname = Malloc(10 + 1);
			if (logname)
				sprintf (logname, "%d", uid);
		}
	}

	if (!logname)
		errno = ENOMEM;
	return (logname);
}

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

#ident	"@(#)libnsl:dial/strsave.c	1.1.1.1"

#include "uucp.h"

/* #include <errno.h>
/* #include <malloc.h>
/* #include <string.h>
/* #include <sys/types.h>
/* #include <sys/stat.h> */

/* copy str into data space -- caller should report errors. */

GLOBAL char *
strsave(str)
register char *str;
{
	register char *rval;

	rval = malloc(strlen(str) + 1);
	if (rval != 0)
		strcpy(rval, str);
	return(rval);
}

/*	Determine if the effective user id has the appropriate permission
	on a file.  Modeled after access(2).
	amode:
		00	just checks for file existence.
		04	checks read permission.
		02	checks write permission.
		01	checks execute/search permission.
		other bits are ignored quietly.
*/

GLOBAL int
eaccess( path, amode )
char		*path;
register mode_t	amode;
{
	struct stat	s;
	uid_t euid;

	if( stat( path, &s ) == -1 )
		return(-1);		/* can't stat file */
	amode &= 07;

	if( (euid = geteuid()) == 0 )
	    return(0);			/* root can do all */
	if( euid == s.st_uid )
	    s.st_mode >>= 6;		/* use owner bits */
	else if( getegid() == s.st_gid )
	    s.st_mode >>= 3;		/* use group bits */

	if( (amode & s.st_mode) == amode )
		return(0);		/* access permitted */
	errno = EACCES;
	return(-1);
}

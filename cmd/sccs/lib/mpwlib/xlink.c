/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sccs:lib/mpwlib/xlink.c	6.3"
/*
	Interface to link(II) which handles all error conditions.
	Returns 0 on success,
	fatal() on failure.
*/

# include	"errno.h"
int
xlink(f1,f2)
char	*f1, *f2;
{
	extern errno;
	extern char Error[];
	int	link(), sprintf(), fatal(), xmsg();

	if (link(f1,f2)) {
		if (errno == EEXIST || errno == EXDEV) {
			sprintf(Error,"can't link `%s' to `%s' (%d)",
				f2,f1,errno == EEXIST ? 111 : 112);
			return(fatal(Error));
		}
		if (errno == EACCES)
			f1 = f2;
		return(xmsg(f1,"xlink"));
	}
	return(0);
}

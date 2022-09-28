/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sccs:lib/mpwlib/xopen.c	6.4"
/*
	Interface to open(II) which differentiates among the various
	open errors.
	Returns file descriptor on success,
	fatal() on failure.
*/

# include "errno.h"
# include <ccstypes.h>

xopen(name,mode)
char name[];
mode_t mode;
{
	register int fd;
	extern int errno;
	extern char Error[];
	int	open(), sprintf(), fatal(), xmsg();

	if ((fd = open(name,mode)) < 0) {
		if(errno == EACCES) {
			if(mode == 0)
				sprintf(Error,"`%s' unreadable (ut5)",name);
			else if(mode == 1)
				sprintf(Error,"`%s' unwritable (ut6)",name);
			else
				sprintf(Error,"`%s' unreadable or unwritable (ut7)",name);
			fd = fatal(Error);
		}
		else
			fd = xmsg(name,"xopen");
	}
	return(fd);
}

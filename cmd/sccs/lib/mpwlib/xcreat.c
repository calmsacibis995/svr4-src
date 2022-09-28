/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sccs:lib/mpwlib/xcreat.c	6.4"
# include	"../../hdr/defines.h"
# include	<ccstypes.h>


/*
	"Sensible" creat: write permission in directory is required in
	all cases, and created file is guaranteed to have specified mode
	and be owned by effective user.
	(It does this by first unlinking the file to be created.)
	Returns file descriptor on success,
	fatal() on failure.
*/
int
xcreat(name,mode)
char *name;
mode_t mode;
{
	register int fd;
	char d[FILESIZE];
	int	xmsg(), creat(), unlink(), fatal(), stat();

	copy(name,d);
	if (!exists(dname(d))) {
		sprintf(Error,"directory `%s' nonexistent (ut1)",d);
		fatal(Error);
	}
	unlink(name);
	if ((fd = creat(name,mode)) >= 0)
		return(fd);
	return(xmsg(name,"xcreat"));
}

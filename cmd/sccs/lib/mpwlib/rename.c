/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sccs:lib/mpwlib/rename.c	6.5"
# include "errno.h"
# include "fatal.h"
# include "signal.h"
/*
	rename (unlink/link)
	Calls xlink() and xunlink().
*/

rename(oldname,newname)
char *oldname, *newname;
{
	extern int errno;
	void (* holdsig[3])();
	int retval;
	int	xunlink(), xlink(), unlink();
	
	/*Ignor signals 01 02 03 */
	holdsig[0] = signal(SIGHUP,SIG_IGN);
	holdsig[1] = signal(SIGINT,SIG_IGN);
	holdsig[2] = signal(SIGQUIT,SIG_IGN);
	if (unlink(newname) < 0 && errno != ENOENT)
		retval = xunlink(newname);

	if (xlink(oldname,newname) == Fvalue)
		retval = -1;
	retval = (xunlink(oldname));
	/*re establish signals */
	signal(SIGHUP,holdsig[0]);
	signal(SIGINT,holdsig[1]);
	signal(SIGQUIT,holdsig[2]);
	return(retval);
}

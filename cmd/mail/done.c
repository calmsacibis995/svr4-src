/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:done.c	1.5.3.1"
#include "mail.h"
/*
	clean up lock files and exit
*/
void done(needtmp)
int	needtmp;
{
	static char pn[] = "done";
	unlock();
	if (!maxerr) {
		maxerr = error;
		Dout(pn, 0, "maxerr set to %d\n", maxerr);
		if ((debug > 0) && (keepdbgfile == 0)) {
			unlink (dbgfname);
		}
	}
	if (maxerr && sending)
		mkdead();
	if (tmpf)
		fclose(tmpf);
	if (!needtmp && lettmp) {
		unlink(lettmp);
	}
	exit(maxerr);
	/* NOTREACHED */
}

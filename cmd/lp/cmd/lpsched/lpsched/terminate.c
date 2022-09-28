/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:cmd/lpsched/terminate.c	1.2.4.1"

#include "lpsched.h"

/**
 ** terminate() - STOP A CHILD PROCESS
 **/

void
#if	defined(__STDC__)
terminate (
	register EXEC *		ep
)
#else
terminate (ep)
	register EXEC		*ep;
#endif
{
	ENTRY ("terminate")

	if (ep->pid > 0) {

#if	defined(DEBUG)
		if (debug & DB_EXEC)
			execlog (
				"KILL: pid %d%s%s\n",
				ep->pid,
				((ep->flags & EXF_KILLED)?
					  ", second time"
					: ""
				),
				(kill(ep->pid, 0) == -1?
					  ", but child is GONE!"
					: ""
				)
			);
#endif

		if (ep->flags & EXF_KILLED)
			return;
		ep->flags |= EXF_KILLED;

		/*
		 * Theoretically, the following "if-then" is not needed,
		 * but there's some bug in the code that occasionally
		 * prevents us from hearing from a finished child.
		 * (Kill -9 on the child would do that, of course, but
		 * the problem has occurred in other cases.)
		 */
		if (kill(-ep->pid, SIGTERM) == -1 && errno == ESRCH) {
			ep->pid = -99;
			ep->status = SIGTERM;
			ep->errno = 0;
			DoneChildren++;
		}
	}
	return;
}
